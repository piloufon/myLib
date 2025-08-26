#pragma once
#include "include.h"

namespace DX12 {
	class Device {
	public:
		bool InitializeDevice();
		void Shutdown();

		// TODO : Add DirectX 12 Agility SDK -> If I understand correctly let the use of DirectX 12.5/12.6 on Windows 10 1909+
		inline bool CheckFeature(D3D12_FEATURE feature, void* featureSupportData, UINT featureSupportDataSize);

		inline ComPtr<ID3D12Device10> GetpDevice() { return device; }
	private:
		ComPtr<ID3D12Device10> device;
	};
}

