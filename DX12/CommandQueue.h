#pragma once
#include "include.h"

namespace DX12 {
	constexpr UINT8 INVALID_CONTEXT_INDEX = UCHAR_MAX;

	class CommandQueue {
		struct AllocatorContext {
			ComPtr<ID3D12CommandAllocator> allocator;
			std::vector<ComPtr<ID3D12GraphicsCommandList10>> cmdLists;
			std::vector<ComPtr<ID3D12Resource>> keepResources;
			UINT64 lastFenceValue = 0;
		};

	public:
		bool InitializeCommandQueue(ComPtr<ID3D12Device10> device);
		void Shutdown();

		[[nodiscard]] UINT8 GetAllocatorContextIndex();

		ComPtr<ID3D12GraphicsCommandList10> StartRecording(UINT8 contextIndex, D3D12_COMMAND_LIST_TYPE typeCmdList = D3D12_COMMAND_LIST_TYPE_DIRECT);
		//void AddCopyCommand(UINT8 contextIndex /*, copy params */);
		//void AddResource(UINT8 contextIndex, ComPtr<ID3D12Resource> resource);
		void Finalize(UINT8 contextIndex);

		void ProcessCommandQueue();
	private:
		std::mutex mtx;
		std::condition_variable cv;
		std::atomic<bool> wakeFlag = false;
		std::atomic<bool> shouldStop = false;  // Add this
		std::thread cmdQueueWorker;
		
		void ProcessCommandQueueWorker();
		void ExecuteFinishedContexts();
		void CleanAllocatorContext();

		ComPtr<ID3D12Device10> device;
		ComPtr<ID3D12Fence1> fence;
		UINT64 fenceValue = 0;
		HANDLE fenceEvent = nullptr;

		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		ComPtr<ID3D12CommandQueue> cmdQueue;


		std::array<AllocatorContext, cmdAllocatorCount> poolAllocatorContext;

		std::queue<UINT8> freeAllocatorContext;
		std::queue<UINT8> finishedAllocatorContext;
		std::queue<UINT8> allocatorContextToClean;

		// TODO : Create a pool of "urgent" commandAllocator that needs to be immediatly execute -> Swapchain et caetera...
	};
}

