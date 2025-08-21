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
	class DescriptorManager {
	public:
		bool InitializeDescriptorHeapManager(ComPtr<ID3D12Device10> device);
		void Shutdown();

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
	private:
		static constexpr UINT singleDevice = 0;   // Read https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_heap_desc
		static constexpr UINT8 maxRTV = 256;
		static constexpr UINT8 maxDSV = 32;

		ComPtr<ID3D12Device10> device;

		bool InitRTVs();
		UINT rtvDescriptorSize;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart;
		ComPtr<ID3D12DescriptorHeap> rtvDescHeap;

		bool InitDSVs();
		ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
		UINT dsvDescriptorSize;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapStart;
		
	};
}


