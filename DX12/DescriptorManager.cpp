#include "DescriptorManager.h"

namespace DX12 {
    bool DescriptorManager::InitializeDescriptorHeapManager(ComPtr<ID3D12Device10> device) {
        this->device = device;

        if (!InitRTVs()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - InitializeDescriptorHeapManager", L"Failed to initialize RTV heaps");
            return false;
        }
        if (!InitDSVs()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - InitializeDescriptorHeapManager", L"Failed to initialize DSV heaps");
            return false;
        }
        if (!InitSamplers()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - InitializeDescriptorHeapManager", L"Failed to initialize Sampler heaps");
            return false;
        }

        return true;
    }
    void DescriptorManager::Shutdown() {
        samplerDescHeap.Reset();
        rtvDescHeap.Reset();
        dsvDescHeap.Reset();
        device.Reset();
    }

    bool DescriptorManager::InitRTVs() {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = maxRTV,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = singleDevice
        };

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescHeap)))) [[unlikely]] {
            LOG_WARNING(L"DescriptorManager - InitRTVs", L"Failed to create RTV descriptor heap");
            return false;
        }

        rtvHeapStart = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (UINT8 i = 0; i < maxRTV; i++) {
            freeRTVSlots.push(i);
        }

        return true;
    }
    UINT8 DescriptorManager::AllocateRTV() {
        if (freeRTVSlots.empty()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - AllocateRTV", L"No free RTV slots available");
            return INVALID_DESCRIPTOR_INDEX;
        }

        UINT8 index = freeRTVSlots.front();
        freeRTVSlots.pop();

        return index;
    }
    void DescriptorManager::FreeRTV(UINT8 index) {
        if (index >= maxRTV) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - FreeRTV", L"Invalid RTV index");
            return;
        }

        freeRTVSlots.push(index);
    }

    bool DescriptorManager::InitDSVs() {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = maxDSV,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = singleDevice
        };
        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvDescHeap)))) [[unlikely]] {
            LOG_WARNING(L"DescriptorManager - InitDSVs", L"Failed to create DSV descriptor heap");
            return false;
        }

        dsvHeapStart = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
        dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        for (UINT8 i = 0; i < maxDSV; i++) {
            freeDSVSlots.push(i);
        }

        return true;
    }
    UINT8 DescriptorManager::AllocateDSV() {
        if (freeDSVSlots.empty()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - AllocateDSV", L"No free DSV slots available");
            return INVALID_DESCRIPTOR_INDEX;
        }

        UINT8 index = freeDSVSlots.front();
        freeDSVSlots.pop();

        return index;
    }
    void DescriptorManager::FreeDSV(UINT8 index) {
        if (index >= maxDSV) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - FreeDSV", L"Invalid DSV index");
            return;
        }

        freeDSVSlots.push(index);
    }
    
    bool DescriptorManager::InitSamplers() {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
          .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
          .NumDescriptors = maxSampler,
          .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
          .NodeMask = singleDevice
        };

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&samplerDescHeap)))) [[unlikely]] {
            LOG_WARNING(L"DescriptorManager - InitSamplers", L"Failed to create sampler descriptor heap");
            return false;
        }

        samplerHeapStart = samplerDescHeap->GetCPUDescriptorHandleForHeapStart();
        samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

        for (UINT16 i = 0; i < maxSampler; i++) {
            freeSamplerSlots.push(i);
        }

        return true;
    }
    UINT16 DescriptorManager::AllocateSampler() {
        if (freeSamplerSlots.empty()) [[unlikely]] {
            LOG_ERROR(L"DescriptorManager - AllocateDSV", L"No free Sampler slots available");
            return INVALID_DESCRIPTOR_SAMPLER_INDEX;
        }
        
        UINT16 index = freeSamplerSlots.front();
        freeSamplerSlots.pop();

        return index;
    }
    UINT16 DescriptorManager::CreateSampler(const D3D12_SAMPLER_DESC& desc) {
        UINT16 index = AllocateSampler();
        if (index == INVALID_DESCRIPTOR_SAMPLER_INDEX) {
            return INVALID_DESCRIPTOR_SAMPLER_INDEX;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = GetSamplerHandle(index);
        device->CreateSampler(&desc, handle);

        return index;
    }
}

        