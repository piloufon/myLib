#pragma once
#include "../WindowManager/WindowManager.h"

#include "Device.h"
#include "Factory.h"
#include "CommandQueue.h"
//#include "DescriptorHeapManager.h"
//#include "SwapChain.h"
//#include "BufferManager.h"

namespace WindowManager {class WindowManager; }

namespace DX12 {
    
    class DX12 {
    public:
        bool Initialize(WindowManager::WindowManager window);
        void Shutdown();

        void Update();
    private:
        WindowManager::WindowManager mainWindow;

        Factory                 dxFactory;
        Device                  dxDevice;
        CommandQueue            dxCommandQueue;
        //DescriptorHeapManager   dxDescriptorHeapManager;
        //SwapChain               dxSwapChain;
        //BufferManager           dxBufferManager;
    };

}