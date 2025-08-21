#pragma once
#include "../WindowManager/WindowManager.h"

#include "Device.h"
#include "Factory.h"
#include "CommandQueue.h"
#include "DescriptorManager.h"

namespace WindowManager {class WindowManager; }

namespace DX12 {
    
    class DX12 {
    public:
        bool Initialize(WindowManager::WindowManager* window);
        void Shutdown();

        void Update();
    private:
        WindowManager::WindowManager* mainWindow;

        DescriptorManager   dxDescriptorManager;
        Factory                 dxFactory;
        Device                  dxDevice;
        CommandQueue            dxCommandQueue;
    };

}