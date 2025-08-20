#include "DX12.h"
#pragma comment(lib, "WindowManager.lib")

namespace DX12 {
	bool DX12::Initialize(WindowManager::WindowManager window) {
		mainWindow = window;

		UINT factoryFlag = 0;
		#ifdef _DEBUG
			factoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
		#endif
		if (!dxFactory.InitializeFactory(0) || !dxDevice.InitializeDevice()) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize either dxDevice or dxDevice");
			return false;
		}
		if (!dxCommandQueue.InitializeCommandQueue(dxDevice.GetpDevice())) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxCommandQueue");
			return false;
		}
		/*if (!dxDescriptorHeapManager.InitializeDescriptorHeapManager(dxDevice.GetpDevice())) [[unlikely]]  {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize dxDescriptorHeapManager");
			return false;
		}
		if (!dxSwapChain.InitializeSwapChain(dxFactory.GetpFactory(), dxCommandQueue.GetpCommandQueue(), mainWindow)) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't initialize InitializeSwapChain");
			return false;
		}
		if (!dxSwapChain.SetupBuffers(dxDevice.GetpDevice(), dxDescriptorHeapManager.GetRTVHandles())) [[unlikely]] {
			LOG_FATAL(L"DX12 - Initialize", L"Coulnd't use SetupBuffers");
			return false;
		}*/


		return true;
	}

	void DX12::Shutdown() {
		//dxBufferManager.Shutdown();
		//dxSwapChain.Shutdown();
		//dxDescriptorHeapManager.Shutdown();
		dxCommandQueue.Shutdown();
		dxDevice.Shutdown();
		dxFactory.Shutdown();
	}

	void DX12::Update() {
		UINT8 contextIndex = dxCommandQueue.GetAllocatorContextIndex();
		if (contextIndex == INVALID_CONTEXT_INDEX) {
			return;
		}

		auto cmdList = dxCommandQueue.StartRecording(contextIndex, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!cmdList) {
			return;
		}

		auto cmdList2 = dxCommandQueue.StartRecording(contextIndex, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (cmdList2) {
			cmdList2->SetGraphicsRootSignature(nullptr);
		}

		dxCommandQueue.Finalize(contextIndex);


		dxCommandQueue.ExecuteFinishedContexts();
	}
}