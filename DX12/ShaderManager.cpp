#include "ShaderManager.h"
#include "..\FileManager\FileManager.h"

namespace DX12 {
	ShaderManager::ShaderManager() {
		constexpr DWORD bufSize = MAX_PATH;
		alignas(16) WCHAR buffer[bufSize];

		DWORD size = GetModuleFileNameW(nullptr, buffer, bufSize);
		if (size == 0) {
			LOG_FATAL(L"ShaderManger - ShaderManger", L"Couldn't use GetModuleFileNameW, killing process...");
		}

		WCHAR* lastSlash = nullptr;
		for (WCHAR* p = buffer; *p; ++p) {
			if (*p == L'\\') lastSlash = p;
		}

		pathToShader = std::wstring(buffer, lastSlash ? lastSlash - buffer : size);
	}
	ShaderManager::ShaderManager(const std::string path) {
		pathToShader = std::wstring(path.begin(), path.end());
	}
	
	bool ShaderManager::LoadShader(const std::string& name) {
		if (pathToShader.empty()) [[unlikely]] {
			LOG_ERROR(L"ShaderManager - LoadShader", L"pathToShader is empty");
			return false;
		}
		
		std::wstring fullPath = pathToShader + L"\\" + std::wstring(name.begin(), name.end());

		FileManager::FileManager fm;
		if (!fm.FileExists(fullPath)) [[unlikely]] {
			LOG_WARNING(L"ShaderManager - LoadShader", L"File (" + fullPath + L") doesn't exist");
			return false;
		}
		if (!fm.ReadFile(fullPath)) [[unlikely]] {
			LOG_WARNING(L"ShaderManager - LoadShader", L"File (" + fullPath + L") couldn't be readed");
			return false;
		}

		cachedShader[name] = fm.MoveData();
		if (cachedShader[name].empty()) [[unlikely]] {
			LOG_WARNING("ShaderManager - LoadShader", "Shader (" + name + ") is empty");
		}

		return true;
	}
	bool ShaderManager::UnloadShader(const std::string& name) {
		if (!cachedShader.contains(name)) {
			LOG_WARNING("ShaderManager - UnloadShader", "cachedShader doesn't contains " + name);
			return false;
		}
		cachedShader.erase(name);

		return true;
	}
}
