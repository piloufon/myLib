#pragma once
#include "include.h"

namespace DX12 {
	class ShaderManager {
	public:
		ShaderManager();
		ShaderManager(const std::string path);

		bool LoadShader(const std::string& name);
		bool UnloadShader(const std::string& name);

		inline const std::vector<UINT8>& GetShader(const std::string& name) {
			auto it = cachedShader.find(name);
			if (it == cachedShader.end()) {
				LOG_ERROR(L"ShaderManager - GetShader", L"Shader doesn't exist");
			}
			return it->second;
		}
	private:
		std::wstring pathToShader;

		std::unordered_map<std::string, std::vector<UINT8>> cachedShader;
	};
}
