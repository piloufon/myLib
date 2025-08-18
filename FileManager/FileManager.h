#pragma once
#include "include.h"
#include "..\LogManager\LogManager.h"

namespace FileManager {
    
	class FileManager {
	public:
		bool ReadFile(const wstring& filePath, UINT64 offset = 0, UINT64 offsetEnd = 0);
		bool ReadFile(const string& filePath, UINT64 offset = 0, UINT64 offsetEnd = 0);

		bool WriteFile(const wstring& filePath, const vector<UINT8>& dataToWrite, UINT64 offset = 0);
		bool EraseSection(const wstring& filePath, UINT64 offset, UINT64 offsetEnd = 0);

		bool FileExists(const wstring& filePath);
		bool DirectoryExists(const wstring& filePath);
		bool HasFiles(const wstring& dirPath);


		bool CreateFile(wstring& dirPath, wstring& name);
		bool DeleteFile(const wstring& filePath);

		bool CreateDirectory(const wstring& dirPath, const wstring& dirName);
		bool DeleteDirectory(const wstring& dirPath);
		// Create a safe method and an unsafe -> here I just have to create EnsureCreateDirectorry that will force 
		//	it's execution even by deleting everything (with still a MessageBox asking if sure)

		

		vector<UINT8> GetData() { return data; }
		constexpr vector<UINT8> MoveData() { return move(data); }
	private:
		vector<UINT8> data;
	};

}
