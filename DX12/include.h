#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#ifdef _DEBUG
	#include <d3d12sdklayers.h>
	#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <wrl.h>

using Microsoft::WRL::ComPtr;

#include "..\LogManager\LogManager.h"

static constexpr UINT8 bufferCount = 3;
static constexpr UINT8 cmdAllocatorCount = 16;

#include <unordered_map>
#include <vector>
#include <map>
#include <queue>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
