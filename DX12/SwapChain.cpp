#include "SwapChain.h"
#include "DescriptorManager.h"
#include "../WindowManager/WindowManager.h"
#pragma comment(lib, "WindowManager.lib")

namespace DX12 {
	bool SwapChain::InitializeSwapChain(ComPtr<IDXGIFactory7> factory, ComPtr<ID3D12CommandQueue> cmdQueue, WindowManager::WindowManager* window) {
		if (!factory) [[unlikely]] {
			LOG_ERROR(L"SwapChain - InitializeSwapChain", L"ComPtr<IDXGIFactory7> factory = nullptr");
			return false;
		}
		if (!cmdQueue) [[unlikely]] {
			LOG_ERROR(L"SwapChain - InitializeSwapChain", L"ComPtr<ID3D12Device10> device = nullptr");
			return false;
		}
		if (window == nullptr) [[unlikely]] {
			LOG_ERROR(L"SwapChain - InitializeSwapChain", L"WindowManager::WindowManager* window = nullptr");
			return false;
		}

		// Thanks : Lötwig Fusel (ytb)
		static const DXGI_SWAP_CHAIN_DESC1 scDesc{
			.Width = window->GetWidth(),
			.Height = window->GetHeight(),
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc{ // Check if MSAA is supported (D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS + device->CheckFeatureSupport)
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = bufferCount,
			.Scaling = DXGI_SCALING_NONE, // TODO : I will probably use a dedicated thread to handle the resize (prevent stretch + very little more "performance" than DXGI_SCALING_STRETCH)
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE, // If I want transparancy => ALPHA_MODE_PREMULTIPLIED
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
		};

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fScDesc{ // TODO
			.Windowed = true,
		};

		ComPtr<IDXGISwapChain1> version1SwapChain; // CreateSwapChain take IDXGISwapChain1 not IDXGISwapChain3
		if (FAILED(factory->CreateSwapChainForHwnd(cmdQueue.Get(), window->GetHWnd(), &scDesc, &fScDesc, nullptr, &version1SwapChain))) [[unlikely]] {
			LOG_ERROR(L"SwapChain - InitializeSwapChain", L"Couldn't execute CreateSwapChainForHwnd");
			return false;
		}

		if (FAILED(version1SwapChain.As(&swapChain))) [[unlikely]] {
			LOG_ERROR(L"SwapChain - InitializeSwapChain", L"Couldn't convert version1SwapChain in swapChain");
			version1SwapChain.Reset();
			return false;
		}

		version1SwapChain.Reset();

		return true;
	}
	void SwapChain::Shutdown(DescriptorManager& descriptorMgr) {
		ReleaseBuffer(descriptorMgr);
		swapChain.Reset();
	}

	bool SwapChain::CreateBuffer(ComPtr<ID3D12Device10> device, DescriptorManager& descriptorMgr) {
		ReleaseBuffer(descriptorMgr);
		
		for (UINT i = 0; i < bufferCount; i++) {
			if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffers[i])))) [[unlikely]] {
				LOG_ERROR(L"SwapChain - CreateBuffer", L"Couldn't execute swapChain->GetBuffer(...)");
				ReleaseBuffer(descriptorMgr);
				return false;
			}
			rtvIndices[i] = descriptorMgr.AllocateRTV();
			device->CreateRenderTargetView(buffers[i].Get(), &rtv, descriptorMgr.GetRTVHandle(rtvIndices[i]));
		}
		
		return true;

	}
	void SwapChain::ReleaseBuffer(DescriptorManager& descriptorMgr) {
		for (auto& buff : buffers) {
			buff.Reset();
		}
		for (const UINT8 i : rtvIndices) {
			descriptorMgr.FreeRTV(i);
		}
	}

	void SwapChain::BeginFrame(ComPtr<ID3D12GraphicsCommandList10> cmdList) {
		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
		
		D3D12_RESOURCE_BARRIER barr{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition{
			.pResource = buffers[currentBackBufferIndex].Get(),
			.Subresource = 0,
			.StateBefore = D3D12_RESOURCE_STATE_PRESENT,
			.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET
			}
		};

		cmdList->ResourceBarrier(1, &barr);
	}
	void SwapChain::EndFrame(ComPtr<ID3D12GraphicsCommandList10> cmdList) {
		if (!cmdList) [[unlikely]] {
			LOG_ERROR(L"SwapChain - EndFrame", L"cmdList is nullptr");
			return;
		}
		
		D3D12_RESOURCE_BARRIER barr{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition{
			.pResource = buffers[currentBackBufferIndex].Get(),
			.Subresource = 0,
			.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
			.StateAfter = D3D12_RESOURCE_STATE_PRESENT
			}
		};

		cmdList->ResourceBarrier(1, &barr);
	}
}