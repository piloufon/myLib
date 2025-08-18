#include "FileManager.h"
#include "..\..\myLib\LogManager\LogManager.h"

namespace FileManager {

    bool FileManager::ReadFile(const string& filePath, UINT64 offset, UINT64 offsetEnd) {
        wstring path = wstring(filePath.begin(), filePath.end());
        return ReadFile(path, offset, offsetEnd);
    }
    bool FileManager::ReadFile(const wstring& filePath, UINT64 offset, UINT64 offsetEnd) {
        if (!FileExists(filePath)) {
            LOG_WARNING(L"FileManager - ReadFile", L"(" + filePath + L") File doesn't exist");
            return false;
        }

        // Start : Get HANDLE 
        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - ReadFile", "CreateFile");
            return false;
        }
        // End : Get HANDLE

        // Start : Get Size + Checking
        LARGE_INTEGER fileSizeStruct;
        if (!GetFileSizeEx(hFile, &fileSizeStruct)) {
            LOG_ERROR("FileManager - ReadFile", "Error with GetFileSizeEx");
            CloseHandle(hFile);
            return false;
        }


        UINT64 fileSize = fileSizeStruct.QuadPart;

        if (fileSize < offset) {
            LOG_WARNING("FileManager - ReadFile", "Starting offset is bigger than the file size : "
                + to_string(fileSize) + " < " + to_string(offset));
            CloseHandle(hFile);
            return false;
        }
        if (fileSize < offsetEnd) {
            LOG_WARNING("FileManager - ReadFile", "Ending offset is bigger than the file size : "
                + to_string(fileSize) + " < " + to_string(offsetEnd));
            CloseHandle(hFile);
            return false;
        }

        offsetEnd = offsetEnd == 0 ? fileSize : offsetEnd;

        if (offset > offsetEnd) {
            LOG_WARNING("FileManager - ReadFile", "Ending offset is bigger than the starting offset : "
                + to_string(offset) + " > " + to_string(offsetEnd));
            CloseHandle(hFile);
            return false;
        }

        UINT64 bytesToRead = offsetEnd - offset;

        if (bytesToRead > MAXDWORD) {
            LOG_WARNING("FileManager - ReadFile", "File to big (> 4GB)");
            CloseHandle(hFile);
            return false;
        }
        // End : Get Size + Checking


        data.clear();
        data.resize(bytesToRead);

        DWORD bytesRead = 0;

        OVERLAPPED overlapped = { 0 };
        overlapped.Offset = (DWORD)(offset & 0xFFFFFFFF);        // For 4  firsts bytes of the UINT64 offset
        overlapped.OffsetHigh = (DWORD)(offset >> 32);           // For 4 lastest bytes of the UINT64 offset

        BOOL success = ::ReadFile(
            hFile,
            data.data(),
            static_cast<DWORD>(bytesToRead),
            &bytesRead,
            &overlapped
        );

        CloseHandle(hFile);

        if (!success || bytesRead != bytesToRead) {
            LOG_ERROR("FileManager - ReadFile", "ReadFile - Readed: " + to_string(bytesRead) +
                " / Should've been: " + to_string(bytesToRead));
            return false;
        }


        return true;
    }
    bool FileManager::WriteFile(const wstring& filePath, const vector<UINT8>& dataToWrite, UINT64 offset) {
        if (!FileExists(filePath)) {
            LOG_WARNING(L"FileManager - WriteFile", L"(" + filePath + L") File doesn't exist");
            return false;
        }

        // Start : Get HANDLE 
        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - WriteFile", "CreateFile");
            return false;
        }
        // End : Get HANDLE 


        // Start : Get Size
        LARGE_INTEGER fileSizeStruct;
        if (!GetFileSizeEx(hFile, &fileSizeStruct)) {
            LOG_ERROR("FileManager - WriteFile", "Error with GetFileSizeEx");
            CloseHandle(hFile);
            return false;
        }

        UINT64 fileSize = fileSizeStruct.QuadPart;

        OVERLAPPED overlapped = { 0 };
        overlapped.Offset = (DWORD)(offset & 0xFFFFFFFF);        // For 4  firsts bytes of the UINT64 offset
        overlapped.OffsetHigh = (DWORD)(offset >> 32);           // For 4 lastest bytes of the UINT64 offset

        if (dataToWrite.size() > MAXDWORD) {
            LOG_WARNING("FileManager - WriteFile", "Data to big (> 4GB)");
            CloseHandle(hFile);
            return false;
        }

        DWORD bytesToWrite = static_cast<DWORD>(dataToWrite.size());
        // End : Get Size

        DWORD bytesWritten = 0;

        bool success = ::WriteFile(
            hFile,
            dataToWrite.data(),
            bytesToWrite,
            &bytesWritten,
            &overlapped
        );

        CloseHandle(hFile);

        if (!success || bytesWritten != bytesToWrite) {
            LOG_WARNING("FileManager - WriteFile", "Error with WriteFile - Written: " + to_string(bytesWritten) +
                " / Should've been: " + to_string(bytesToWrite));
            return false;
        }

        return true;
    }
    bool FileManager::EraseSection(const wstring& filePath, UINT64 offset, UINT64 offsetEnd) {
        if (!FileExists(filePath)) {
            LOG_WARNING(L"FileManager - EraseSection", L"(" + filePath + L") File doesn't exist");
            return false;
        }

        vector<UINT8> someData;
        // Start : Get HANDLE 
        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - EraseSection", "CreateFile");
            return false;
        }
        // End : Get HANDLE 

        // Start : Get Size + Checking
        LARGE_INTEGER fileSizeStruct;
        if (!GetFileSizeEx(hFile, &fileSizeStruct)) {
            LOG_ERROR("FileManager - EraseSection", "Error with GetFileSizeEx");
            CloseHandle(hFile);
            return false;
        }


        UINT64 fileSize = fileSizeStruct.QuadPart;

        if (fileSize < offset) {
            LOG_WARNING("FileManager - EraseSection", "Starting offset is bigger than the file size : "
                + to_string(fileSize) + " < " + to_string(offset));
            CloseHandle(hFile);
            return false;
        }
        if (fileSize < offsetEnd) {
            LOG_WARNING("FileManager - EraseSection", "Ending offset is bigger than the file size : "
                + to_string(fileSize) + " < " + to_string(offsetEnd));
            CloseHandle(hFile);
            return false;
        }

        bool shouldCopy = offsetEnd == 0 ? false : true;

        if (offset > offsetEnd && offsetEnd != 0) {
            LOG_WARNING("FileManager - EraseSection", "Ending offset is bigger than the starting offset : "
                + to_string(offset) + " > " + to_string(offsetEnd));
            CloseHandle(hFile);
            return false;
        }

        offsetEnd = offsetEnd == 0 ? fileSize : offsetEnd;

        // End : Get Size

        if (shouldCopy) {
            FileManager fm;

            if (!fm.ReadFile(filePath, offsetEnd)) {
                LOG_ERROR("FileManager - EraseSection", "FileManager::ReadFile failed");
                return false;
            }
            someData = fm.MoveData();
        }

        LARGE_INTEGER offsetStruct;
        offsetStruct.QuadPart = offset;
        bool success = SetFilePointerEx(hFile, offsetStruct, nullptr, FILE_BEGIN);
        if (!success) LOG_ERROR(L"FileManager - EraseSection", L"SetFilePointerEx failed at offset " + to_wstring(offset));

        success = SetEndOfFile(hFile);
        if (!success) LOG_ERROR(L"FileManager - EraseSection", L"SetEndOfFile failed at offset " + to_wstring(offset) + L"with hFile " + to_wstring((DWORD)hFile));


        if (shouldCopy) {
            FileManager fm;

            if (!fm.WriteFile(filePath, someData, offset)) {
                LOG_INFO("FileManager - EraseSection", "FileManager::WriteFile failed");
                return false;
            }
        }

        return true;
    }

    bool FileManager::FileExists(const wstring& filePath) {
        DWORD attributes = GetFileAttributesW(filePath.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES &&
            !(attributes & FILE_ATTRIBUTE_DIRECTORY));
    }
    bool FileManager::DirectoryExists(const wstring& dirPath) {
        DWORD attributes = GetFileAttributesW(dirPath.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY));
    }
    bool FileManager::HasFiles(const wstring& dirPath) {
        if (!DirectoryExists(dirPath)) {
            LOG_WARNING("FileManager - HasFiles", "Directory doesn't exist");
            return false;
        }

        wstring searchPath = dirPath;
        if (searchPath.back() != L'\\') {
            searchPath += L'\\';
        }
        searchPath += L"*";

        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

        if (hFind == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - HasFiles", "FindFirstFileW");
            return false;
        }

        bool hasContent = false;

        while (FindNextFileW(hFind, &findData) != 0) {
            if (wcscmp(findData.cFileName, L".") == 0 ||
                wcscmp(findData.cFileName, L"..") == 0) {
                continue;
            }

            hasContent = true;
            break;

        }


        FindClose(hFind);
        return hasContent;
    }


    bool FileManager::CreateFile(wstring& dirPath, wstring& name) {
        if (dirPath.empty()) {
            LOG_WARNING("FileManager - CreateFile", "Directory is empty");
            return false;
        }
        if (name.empty()) {
            LOG_ERROR("FileManager - CreateFile", "File name is empty");
            return false;
        }

        if (!DirectoryExists(dirPath)) {
            LOG_WARNING(L"FileManager - CreateFile", L"Directory (" + dirPath + L") doesn't exist");
            return false;
        }


        // Clean filePath
        if (dirPath.back() != L'\\') {
            dirPath += L'\\';
        }
        if (name[0] == L'\\') {
            name.erase(0, 1);
        }
        wstring filePath = dirPath + name;

        if (FileExists(filePath)) {
            LOG_WARNING(L"FileManager - CreateFile", L"File (" + filePath + L") already exist");
            return false;
        }

        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - CreateFile", "CreateFileW");
            return false;
        }

        CloseHandle(hFile);
        return true;
    }
    bool FileManager::DeleteFile(const wstring& filePath) {
        if (!FileExists(filePath)) {
            LOG_WARNING(L"FileManager - DeleteFile", L"File (" + filePath + L") doesn't exist");
            return false;
        }

        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            DELETE,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_DELETE_ON_CLOSE,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            LOG_ERROR("FileManager - DeleteFile", "CreateFileW");
            return false;
        }

        CloseHandle(hFile);

        if (FileExists(filePath)) {
            LOG_ERROR("FileManager - DeleteFile", "File still exist");
            return false;
        }

        return true;
    }


    bool FileManager::CreateDirectory(const wstring& path, const wstring& dirName) {
        if (!DirectoryExists(path)) {
            LOG_WARNING("FileManager - CreateDirectory", "Parent directory doesn't exist");
            return false;
        }

        wstring dirPath = path;
        if (!path.empty() && path.back() != L'\\' &&
            !dirName.empty() && dirName[0] != L'\\') {
            dirPath += L'\\';
        }
        dirPath += (dirName[0] == L'\\') ? dirName.substr(1) : dirName;

        if (DirectoryExists(dirPath)) {
            LOG_WARNING("FileManager - CreateDirectory", "Directory already exists");
            return false;
        }
        if (FileExists(dirPath)) {
            LOG_ERROR("FileManager - CreateDirectory", "A file with the same name already exists");
            return false;
        }

        BOOL success = CreateDirectoryW(
            dirPath.c_str(),
            nullptr
        );

        if (!success) {
            LOG_ERROR("FileManager - CreateDirectory","CreateDirectoryW");
            return false;
        }

        return true;
    }
    bool FileManager::DeleteDirectory(const wstring& dirPath) {
        if (!DirectoryExists(dirPath)) {
            LOG_WARNING(L"FileManager - DeleteDirectory", L"Directory doesn't exist");
            return false;
        }
        if (HasFiles(dirPath)) {
            LOG_WARNING("FileManager - DeleteDirectory","Has file in exist");
            return false;
        }
        // Check if files inside

        bool success = RemoveDirectoryW(dirPath.c_str());

        if (!success) {
            LOG_ERROR(L"FileManager - DeleteDirectory", L"RemoveDirectoryW failed");
            return false;
        }

        return true;
    }
    }
