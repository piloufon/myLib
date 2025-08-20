#include "CommandQueue.h"

namespace DX12 {
    bool CommandQueue::InitializeCommandQueue(ComPtr<ID3D12Device10> device) {
        this->device = device;

        cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        cmdQueueDesc.NodeMask = 0;
        cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;


        if (FAILED(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue)))) [[unlikely]] {
            LOG_ERROR(L"CommandQueue - InitializeCommandQueue", L"Failed to execute CreateCommandQueue");
            return false;
        }
        for (auto& cmdAllocator : poolAllocatorContext) {
            if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator.allocator)))) [[unlikely]] {
                LOG_ERROR(L"CommandQueue - InitializeCommandQueue", L"Failed to execute CreateCommandQueue");
                return false;
            }
        }
        
        if (FAILED(device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) [[unlikely]] {
            LOG_ERROR(L"CommandQueue - InitializeCommandQueue", L"Failed to use CreateFence");
            return false;
        }
        fenceEvent = CreateEventW(nullptr, false, false, nullptr);

        for (UINT8 i = 0; i < poolAllocatorContext.size(); i++) {
            freeAllocatorContext.push(i);
        }

        cmdQueueWorker = std::thread(ProcessCommandQueueWorker);

        LOG_INFO(L"CommandQueue - InitializeCommandQueue", L"Success in initializing DX12");
        return true;
    }

    void CommandQueue::Shutdown() {
        shouldStop = true;
        cmdQueueWorker.join();

        if (fenceEvent) {
            CloseHandle(fenceEvent);
        }

        fence.Reset();
        cmdQueue.Reset();

        for (auto& allocatorContext : poolAllocatorContext) {
            allocatorContext.allocator.Reset();
            
            for (auto cmdList : allocatorContext.cmdLists) {
                cmdList.Reset();
            }
            for (auto ressource : allocatorContext.keepResources) {
                ressource.Reset();
            }
        }

        device.Reset();
    }

    UINT8 CommandQueue::GetAllocatorContextIndex() {
        if (freeAllocatorContext.empty()) [[unlikely]] {
            LOG_WARNING(L"CommandQueue - GetAllocatorContext", L"freeAllocatorContext is empty.");
            // TODO : Create a way to create a new allocator for single use only then destroy /or/ make the thread (if possible) wait for some ms to see if an allocator is freed
            return INVALID_CONTEXT_INDEX;
        }
        
        UINT8 result = freeAllocatorContext.front();
        freeAllocatorContext.pop();
        return result;
    }

    ComPtr<ID3D12GraphicsCommandList10> CommandQueue::StartRecording(UINT8 contextIndex, D3D12_COMMAND_LIST_TYPE typeCmdList) {
        if (contextIndex >= poolAllocatorContext.size() || contextIndex == INVALID_CONTEXT_INDEX) [[unlikely]] {
            LOG_ERROR(L"CommandQueue - StartRecording", L"Invalid context index");
            return nullptr;
        }
        auto* context = &poolAllocatorContext[contextIndex];

        // Automatic closing last cmdList 
        if(!context->cmdLists.empty()) {
            HRESULT hr = context->cmdLists.back()->Close();
            if (FAILED(hr) && hr != E_FAIL) [[unlikely]] {
                LOG_ERROR(L"CommandQueue - Finalize", L"Failed to close command list with unexpected error");
            }
        }
        device->CreateCommandList(0, typeCmdList, context->allocator.Get(), nullptr, IID_PPV_ARGS(&context->cmdLists.emplace_back()));

        return context->cmdLists.back();
    }
    void CommandQueue::Finalize(UINT8 contextIndex) {
        if (contextIndex >= poolAllocatorContext.size() || contextIndex == INVALID_CONTEXT_INDEX) [[unlikely]] {
            LOG_ERROR(L"CommandQueue - Finalize", L"Invalid context index");
            return;
        }

        auto* context = &poolAllocatorContext[contextIndex];

        // Depracted
        /*for (auto& cmdList : context->cmdLists) {
            HRESULT hr = cmdList->Close();
            if (FAILED(hr) && hr != E_FAIL) [[unlikely]] {
                LOG_ERROR(L"CommandQueue - Finalize", L"Failed to close command list with unexpected error");
            }
        }*/

        // Only the last cmdList is still open, no need to close the rest
        context->cmdLists.back()->Close();
        finishedAllocatorContext.push(contextIndex);
    }

    void CommandQueue::ProcessCommandQueueWorker() {
        std::unique_lock<std::mutex> lock(mtx);

        ExecuteFinishedContexts();
        CleanAllocatorContext();
    }
    void CommandQueue::ProcessCommandQueue() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            wakeFlag = true;
        }
        
        cv.notify_one();
    }

    void CommandQueue::ExecuteFinishedContexts() {
        if (finishedAllocatorContext.empty()) [[unlikely]] {
            LOG_WARNING(L"CommandQueue - ExecuteFinishedContexts", L"finishedAllocatorContext is empty, cmdQueue won't execute anything");
            return;
        }

        std::vector<ID3D12CommandList*> allCommandLists;
        std::vector<UINT8> processedContexts;

        while (!finishedAllocatorContext.empty()) {
            UINT8 contextIndex = finishedAllocatorContext.front();
            finishedAllocatorContext.pop();

            auto* context = &poolAllocatorContext[contextIndex];

            for (const auto& cmdList : context->cmdLists) {
                allCommandLists.push_back(cmdList.Get());
            }

            processedContexts.push_back(contextIndex);
        }

        if (!allCommandLists.empty()) [[likely]] {
            cmdQueue->ExecuteCommandLists(
                static_cast<UINT>(allCommandLists.size()),
                allCommandLists.data()
            );

            ++fenceValue;
            cmdQueue->Signal(fence.Get(), fenceValue);

            for (UINT8 contextIndex : processedContexts) {
                auto* context = &poolAllocatorContext[contextIndex];
                context->lastFenceValue = fenceValue;
                context->cmdLists.clear();

                allocatorContextToClean.push(contextIndex);
            }
        }
        else [[unlikely]] {
            LOG_WARNING(L"CommandQueue - ExecuteFinishedContexts", L"allCommandLists is empty, cmdQueue won't execute anything (= presence of cmdAllocator but no cmdList)");
            return;
        }
    }
    void CommandQueue::CleanAllocatorContext() {
        while (!allocatorContextToClean.empty()) {
            UINT8 index = allocatorContextToClean.front();
            allocatorContextToClean.pop();

            auto* currentAllocator = &poolAllocatorContext[index];

            if (fence->GetCompletedValue() < currentAllocator->lastFenceValue) {
                break;
            }

            currentAllocator->allocator.Get()->Reset();

            currentAllocator->cmdLists.clear();
            currentAllocator->keepResources.clear();
            currentAllocator->lastFenceValue = 0;

            freeAllocatorContext.push(index);
        }
    }
}