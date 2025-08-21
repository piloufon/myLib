#include "DescriptorManager.h"

namespace DX12 {
    bool DescriptorManager::InitializeDescriptorHeapManager(ComPtr<ID3D12Device10> device) {
        if(!InitRTVs()){
            LOG_ERROR(L"DescriptorHeapManager - InitializeDescriptorHeapManager", L"Failed to execute InitRTVs");
            return false;
        }
        
        return true;
	}
    void DescriptorManager::Shutdown() {
        rtvDescHeap.Reset();
    }

    bool DescriptorManager::InitRTVs() {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = maxRTV,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = singleDevice
        };

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescHeap)))) [[unlikely]] {
            LOG_WARNING(L"DescriptorHeapManager - InitRTVs", L"Failed to create RTV descriptor heap");
            return false;
        }
       
        rtvHeapStart = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        return true;
    }

    bool DescriptorManager::InitDSVs() {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = maxDSV,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = singleDevice
        };

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvDescHeap)))) [[unlikely]] {
            LOG_WARNING(L"DescriptorHeapManager - InitDSVs", L"Failed to create DSV descriptor heap");
            return false;
        }

        dsvHeapStart = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
        dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        return true;
    }
}

        