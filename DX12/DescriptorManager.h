#pragma once
#include "include.h"

// Helper for me
//	
//	enum RTVUsage {
//		BackBuffer,
//		ShadowMap,
//		GBufferTarget,
//		PostProcessTarget
//	};
// enum DSVUsage {
//       MainDepthBuffer,
//       ShadowMapDepth,
//       HiZBuffer
//   };
// 
//

namespace DX12 {
    constexpr UINT8 INVALID_DESCRIPTOR_INDEX = UINT8_MAX;
    constexpr UINT16 INVALID_DESCRIPTOR_SAMPLER_INDEX = UINT16_MAX;

    class DescriptorManager {
    public:
        bool InitializeDescriptorHeapManager(ComPtr<ID3D12Device10> device);
        void Shutdown();

        // Allocation/Deallocation
        [[nodiscard]] UINT8 AllocateRTV();
        void FreeRTV(UINT8 index);
        [[nodiscard]] UINT8 AllocateDSV();
        void FreeDSV(UINT8 index);

        [[nodiscard]] UINT16 AllocateSampler();
        UINT16 CreateSampler(const D3D12_SAMPLER_DESC&); // TODO : Create a way to add D3D12_STATIC_SAMPLER_DESC but in the sahder -> PSO (not now)
        // Handle getters
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(UINT8 index) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeapStart;
            handle.ptr += index * rtvDescriptorSize;
            return handle;
        }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle(UINT8 index) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = dsvHeapStart;
            handle.ptr += index * dsvDescriptorSize;
            return handle;
        }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSamplerHandle(UINT16 index) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = samplerHeapStart;
            handle.ptr += index * samplerDescriptorSize;
            return handle;
        }

    private:
        static constexpr UINT singleDevice = 0;
        static constexpr UINT8 maxRTV = UINT8_MAX;
        static constexpr UINT8 maxDSV = 32;
        static constexpr UINT16 maxSampler = 512; // Max is 2048 -> hardware limit

        ComPtr<ID3D12Device10> device;

        bool InitRTVs();
        UINT rtvDescriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart;
        ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
        std::queue<UINT8> freeRTVSlots;

        bool InitDSVs();
        UINT dsvDescriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapStart;
        ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
        std::queue<UINT8> freeDSVSlots;

        bool InitSamplers();
        ComPtr<ID3D12DescriptorHeap> samplerDescHeap;
        UINT samplerDescriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE samplerHeapStart;        
        std::queue<UINT16> freeSamplerSlots;
    };
}


