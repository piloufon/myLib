#pragma once
#include "include.h"

namespace DX12 {
	class Device {
	public:
		bool InitializeDevice();
		void Shutdown();

		inline bool CheckFeature(D3D12_FEATURE feature, void* featureSupportData, UINT featureSupportDataSize);

		inline ComPtr<ID3D12Device10> GetpDevice() { return device; }
	private:
		ComPtr<ID3D12Device10> device;
	};
}

