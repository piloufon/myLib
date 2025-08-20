#pragma once
#include "include.h"

class DebugLayer {
public:
	static bool InitDebug();

	static void StopDebug();

private:
#ifdef _DEBUG
	static ComPtr<ID3D12Debug6> d3d12Debug;
	static ComPtr<IDXGIDebug1> dxgiDebug;
#endif
};