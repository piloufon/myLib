#pragma once
#include "include.h"

namespace DX12 {
	class Factory {
	public:
		bool InitializeFactory(UINT flag);
		void Shutdown();

		inline ComPtr<IDXGIFactory7> GetpFactory() { return factory; }
	private:
		ComPtr<IDXGIFactory7> factory;
	};
}

