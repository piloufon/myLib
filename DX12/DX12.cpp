#include "DX12.h"
#pragma comment(lib, "WindowManager.lib")


namespace DX12 {
	bool DX12::Initialize(WindowManager::WindowManager* window) {
		mainWindow = window;

		UINT factoryFlag = NULL;
		#ifdef _DEBUG
			factoryFlag = DXGI_CREATE_FACTORY_DEBUG;
		#endif

		const std::string pathToShader = /*path to shader (here is hard coded but will change in the future with the hot shader compilater)*/;
		const std::string rt = "RootSignature.cso";
		const std::string vs = "VertexShader.cso";
		const std::string ps = "PixelShader.cso";
		

		if (!dxFactory.InitializeFactory(factoryFlag) || !dxDevice.InitializeDevice()) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize either dxDevice or dxDevice");
			return false;
		}
		if (!dxCommandQueue.InitializeCommandQueue(dxDevice.GetpDevice())) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxCommandQueue");
			return false;
		}
		if (!dxDescriptorManager.InitializeDescriptorHeapManager(dxDevice.GetpDevice())) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxDescriptorManager");
			return false;
		}
		if (!dxSwapChain.InitializeSwapChain(dxFactory.GetpFactory(), dxCommandQueue.GetpCommandQueue(), window)) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxSwapChain");
			return false;
		}
		if (!dxSwapChain.CreateBuffer(dxDevice.GetpDevice(), dxDescriptorManager)) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't execute dxSwapChain.CreateBuffer");
			return false;
		}
		if (!dxPipelineManager.InitializePipelineManager(dxDevice.GetpDevice(), pathToShader)) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxPipelineManager");
			return false;
		}




		auto& shader = dxPipelineManager.GetShaderManager();
		shader.LoadShader(rt);
		shader.LoadShader(vs);
		shader.LoadShader(ps);

		dxPipelineManager.SetRootSignature(rt);
		dxPipelineManager.CreateMaterialPSO("basicMat", vs, ps);
		
		// Temporary
		dsvIndex = dxDescriptorManager.AllocateDSV();

		return true;
	}

	void DX12::Shutdown() {
		dxCommandQueue.WaitForGPU(); // Meh

		dxPipelineManager.Shutdown();
		dxSwapChain.Shutdown(dxDescriptorManager);
		dxDescriptorManager.Shutdown();
		dxCommandQueue.Shutdown();
		dxDevice.Shutdown();
		dxFactory.Shutdown();
	}

	void DX12::Update() {
		dxCommandQueue.WaitForGPU();

		UINT8 contextIndex = dxCommandQueue.GetAllocatorContextIndex();
		if (contextIndex == INVALID_CONTEXT_INDEX) [[unlikely]] {
			LOG_WARNING(L"DX12 - Update", L"contextIndex is invalid");
			return;
		}
		LOG_DEBUG(L"DX12 - Update", L"contextIndex : " + std::to_wstring(contextIndex));
		ComPtr<ID3D12GraphicsCommandList10> cmdList = dxCommandQueue.StartRecording(contextIndex);


		// Testing purpose
		dxSwapChain.BeginFrame(cmdList);

		//// Set render target
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxDescriptorManager.GetRTVHandle(dxSwapChain.GetCurrentBuffer());
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDescriptorManager.GetDSVHandle(dsvIndex);
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		dxPipelineManager.BindRootSignature(cmdList);
		dxPipelineManager.BindPSO(cmdList, "basicMat");


		const float clearColor[] = { 0.1f, 0.1f, 0.25f, 1.0f };


		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);


		dxSwapChain.EndFrame(cmdList);

	

		dxCommandQueue.Finalize(contextIndex);
		dxCommandQueue.ProcessCommandQueue();
		//Sleep(1); // Block at 60 fps instead of 1000 fps ???

		//dxCommandQueue.ExecuteFinishedContexts();

		dxSwapChain.Present();
		mainWindow->DrawFPS();
		//dxCommandQueue.CleanAllocatorContext();
	}
}