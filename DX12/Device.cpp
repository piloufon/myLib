#include "Device.h"

namespace DX12 {
	bool Device::InitializeDevice() {
		if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) [[unlikely]] {
			LOG_ERROR(L"Device - InitializeDevice", L"Couldn't initialize device");
			return false;
		}
		else [[likely]] return true;
	}

	void Device::Shutdown() {
		device.Reset();
	}

	inline bool Device::CheckFeature(D3D12_FEATURE feature, void* featureSupportData, UINT featureSupportDataSize) {
		if (!device) [[unlikely]] {
			LOG_ERROR(L"Device - CheckFeature", L"Device is null");
			return false;
		}
				
		return SUCCEEDED(device->CheckFeatureSupport(feature, featureSupportData, featureSupportDataSize));
	}
}