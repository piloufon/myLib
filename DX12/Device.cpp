#include "Device.h"

namespace DX12 {
	bool Device::InitializeDevice() {
		if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)))) [[unlikely]] { // TODO (pretty quick but not necessary) : Check if I really need this or if I should check for D3D_FEATURE_LEVEL_11_0
			LOG_ERROR(L"Device - InitializeDevice", L"Couldn't initialize device");
			return false;
		}
		else [[likely]] return true;
	}

	void Device::Shutdown() {
		device.Reset();
	}

	bool Device::CheckFeature(D3D12_FEATURE feature, void* featureSupportData, UINT featureSupportDataSize) {
		if (!device) [[unlikely]] {
			LOG_ERROR(L"Device - CheckFeature", L"Device is null");
			return false;
		}
				
		return SUCCEEDED(device->CheckFeatureSupport(feature, featureSupportData, featureSupportDataSize));
	}
}