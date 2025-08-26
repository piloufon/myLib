#pragma once
#include "include.h"

namespace WindowManager { class WindowManager; }
namespace DX12 { class DescriptorManager; }


namespace DX12 {
	class SwapChain {
	public:
		bool InitializeSwapChain(ComPtr<IDXGIFactory7> factory, ComPtr<ID3D12CommandQueue> cmdQueue, WindowManager::WindowManager* window);
		void Shutdown(DescriptorManager& descriptorMgr);

		bool CreateBuffer(ComPtr<ID3D12Device10> device, DescriptorManager& descriptorMgr);
		void ReleaseBuffer(DescriptorManager& descriptorMgr);

		inline constexpr void Present(bool vsync = false, UINT flags = DXGI_PRESENT_ALLOW_TEARING) { swapChain->Present(vsync, flags * (1 - vsync)); }
		inline UINT8 GetCurrentBuffer() { return currentBackBufferIndex; }

		void BeginFrame(ComPtr<ID3D12GraphicsCommandList10> cmdList);
		void EndFrame(ComPtr<ID3D12GraphicsCommandList10> cmdList);
	private:
		ComPtr<IDXGISwapChain3> swapChain;
		D3D12_RENDER_TARGET_VIEW_DESC rtv {
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM, // DXGI_FORMAT_R16G16B16A16_FLOAT not working bc DXGI is a moron and create while ignoring my parameters
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D {
				.MipSlice = 0,
				.PlaneSlice = 0
				}
		}; // TODO : Learn the documentation well

		std::array<ComPtr<ID3D12Resource2>, bufferCount> buffers = { 0 };
		std::array<UINT8, bufferCount> rtvIndices;

		UINT currentBackBufferIndex = 0;
	};
}