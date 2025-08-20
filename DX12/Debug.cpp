#include "Debug.h"

#ifdef _DEBUG
const GUID DXGI_DEBUG_ALL_CUSTOM = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
#include <d3d12sdklayers.h>

ComPtr<ID3D12Debug6> DebugLayer::d3d12Debug;
ComPtr<IDXGIDebug1> DebugLayer::dxgiDebug;
#endif

bool DebugLayer::InitDebug() {
#ifdef _DEBUG
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
		d3d12Debug->EnableDebugLayer();

		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
			dxgiDebug->EnableLeakTrackingForThread();
			return true;
		}
	}
#endif
	return false;
}

void DebugLayer::StopDebug() {
#ifdef _DEBUG
	if (dxgiDebug) {
		OutputDebugStringW(L"\n");
		OutputDebugStringW(L"DXGI report living device object: \n");

		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL_CUSTOM, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		OutputDebugStringW(L"\n");
	}
	dxgiDebug.Reset();
	d3d12Debug.Reset();
#endif
}