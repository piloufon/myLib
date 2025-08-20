#include "DX12.h"
#pragma comment(lib, "WindowManager.lib")

namespace DX12 {
	bool DX12::Initialize(WindowManager::WindowManager* window) {
		mainWindow = window;

		UINT factoryFlag = NULL;
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

		return true;
	}

	void DX12::Shutdown() {
		dxCommandQueue.Shutdown();
		dxDevice.Shutdown();
		dxFactory.Shutdown();
	}

	void DX12::Update() {
		UINT8 contextIndex = dxCommandQueue.GetAllocatorContextIndex();
		if (contextIndex == INVALID_CONTEXT_INDEX) [[unlikely]] {
			return;
		}

		

		dxCommandQueue.Finalize(contextIndex);
		Sleep(2);

		dxCommandQueue.ProcessCommandQueue();

	}
}