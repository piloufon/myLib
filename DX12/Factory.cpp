#include "Factory.h"

namespace DX12 {
	bool Factory::InitializeFactory(UINT flag) {
		if (FAILED(CreateDXGIFactory2(flag, IID_PPV_ARGS(&factory)))) [[unlikely]] {
			LOG_ERROR(L"Factory - InitializeFactory", L"Couldn't initialize factory");
			return false;
		}
		else [[likely]] return true;
	}

	void Factory::Shutdown() {
		factory.Reset();
	}
}