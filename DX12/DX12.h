#pragma once
#include "../WindowManager/WindowManager.h"

#include "Device.h"
#include "Factory.h"
#include "CommandQueue.h"
#include "DescriptorManager.h"
#include "SwapChain.h"
#include "PipelineManager.h"


namespace WindowManager {class WindowManager; }

namespace DX12 {
    class DX12 {
    public:
        bool Initialize(WindowManager::WindowManager* window);
        void Shutdown();

        void Update();
    private:
        WindowManager::WindowManager* mainWindow;

        Factory                 dxFactory;
        Device                  dxDevice;
        CommandQueue            dxCommandQueue;
        SwapChain               dxSwapChain;
        DescriptorManager       dxDescriptorManager;
        PipelineManager         dxPipelineManager;

		// TODO : Create a queue/vector of lambda that use DSV/RTV/... to be able to create special usage.
		// The queue/vector lambda will be use in the render loop (ex: if I want nothing special only DSV DepthBuffer will be in, but I can push a shadow map lambda for etc...) but accessible at a higher level -> game logics

		UINT8 dsvIndex = 0;
    };

}