#include "LogManager.h"
#include <format>
#include <vector>

#ifdef MessageBox
#undef MessageBox
#endif

namespace LogManager {


#define vbOKOnly 0
#define vbInformation 64
#define vbExclamation 48
#define vbCritical 16

	std::array<LogManager::Action, 5> LogManager::actions = {};
	std::array<std::condition_variable, 5> LogManager::actionCVs = {};
	std::array<bool, 5> LogManager::wakeFlags = {};
	std::array<std::thread, 5> LogManager::actionThreads = {};
	std::array<std::mutex, 5> LogManager::mtxAction = {};
	std::condition_variable LogManager::logCV = {};
	bool LogManager::logWakeFlag = false;
	std::thread LogManager::logThread = {};
	std::mutex LogManager::mtx = {};
	bool LogManager::shouldStop = false;
	std::queue<LogManager::LogInfo> LogManager::logQueue = {};
	std::queue<LogManager::Log> LogManager::debugConsolLog = {};
	std::queue<LogManager::Log> LogManager::messageBoxLog = {};
	std::queue<LogManager::Log> LogManager::writeTruncateLog = {};
	std::queue<LogManager::Log> LogManager::writeAppendLog = {};
	std::queue<LogManager::Log> LogManager::killProcessLog = {};

	LogManager::FileHandle LogManager::hFileTemp;
	LogManager::FileHandle LogManager::hFilePermanent;
	UINT64 LogManager::offsetTempFile = 0;
	UINT64 LogManager::offsetPermFile = 0;

	bool LogManager::initialized = InitalizeAll();
	void LogManager::LogCleanUp() {
		while (!logQueue.empty() || !debugConsolLog.empty() || !messageBoxLog.empty() || !writeTruncateLog.empty() || !writeAppendLog.empty() || !killProcessLog.empty()) {
			if (!logQueue.empty()) {
				logWakeFlag = true;
				logCV.notify_one();
			}
			if (!debugConsolLog.empty()) {
				wakeFlags[0] = true;
				actionCVs[0].notify_one();
			}
			if (!messageBoxLog.empty()) {
				wakeFlags[1] = true;
				actionCVs[1].notify_one();
			}
			if (!writeTruncateLog.empty()) {
				wakeFlags[2] = true;
				actionCVs[2].notify_one();
			}
			if (!writeAppendLog.empty()) {
				wakeFlags[3] = true;
				actionCVs[3].notify_one();
			}
			if (!killProcessLog.empty()) {
				wakeFlags[4] = true;
				actionCVs[4].notify_one();
			}
			Sleep(100);
		}

		shouldStop = true;

		for (unsigned char i = 0; i < 5; i++) {
			wakeFlags[i] = true;
			actionCVs[i].notify_one();
			if (actionThreads[i].joinable()) {
				actionThreads[i].join();
			}
		}

		logWakeFlag = true;
		logCV.notify_one();
		if (logThread.joinable()) {
			logThread.join();
		}
	}
	bool LogManager::InitalizeAll() {
		wakeFlags.fill(false);
		logWakeFlag = false;
		shouldStop = false;

		logThread = std::thread(LogManager::MainLogWorker);

		actionThreads[0] = std::thread(LogManager::DebugConsolWorker);
		actionThreads[1] = std::thread(LogManager::MessageBoxWorker);
		actionThreads[2] = std::thread(LogManager::WriteTruncateWorker);
		actionThreads[3] = std::thread(LogManager::WriteAppendWorker);
		actionThreads[4] = std::thread(LogManager::KillProcessWorker);

		InitializeFileHandles();

		actions[Level::debug] = Action(Action::DEBUG_STRING | Action::MESSAGE_BOX);
		actions[Level::info] = Action::DEBUG_STRING;
		actions[Level::warning] = Action(Action::FILE_TEMP | Action::DEBUG_STRING);
		actions[Level::error] = Action(Action::FILE_TEMP | Action::DEBUG_STRING | Action::MESSAGE_BOX);
		actions[Level::fatal] = Action(Action::MESSAGE_BOX | Action::FILE_PERM | Action::KILL_PROC);

		std::atexit(LogCleanUp);
		return true;
	}

	void LogManager::MainLogWorker() {
		std::unique_lock<std::mutex> lock(mtx);

		while (!shouldStop) {
			logCV.wait(lock, [&] { return LogManager::logWakeFlag || LogManager::shouldStop; });

			if (shouldStop) break;

			if (logWakeFlag) {
				while (!logQueue.empty()) {
					LogInfo log = logQueue.front();
					logQueue.pop();

					std::wstring content = FormatLog(log);
					RedirectLog(log.level, content);
				}
				logWakeFlag = false;
			}
		}
	}
	void LogManager::LOG(const Level level, const std::string source, const std::string message) {
		LOG(level, std::wstring(source.begin(), source.end()), std::wstring(message.begin(), message.end()));
	}
	void LogManager::LOG(const Level level, const std::wstring source, const std::wstring message) {
		{
			std::unique_lock<std::mutex> lock(mtx);
			logQueue.emplace(LogInfo{ level, std::chrono::system_clock::now(), GetLastError(), L" [" + source + L"] " + message });
			logWakeFlag = true;
		}

		logCV.notify_one();
	}
	std::wstring LogManager::FormatLog(const LogInfo& loginfo) {
		auto timePoint = std::chrono::time_point_cast<std::chrono::seconds>(loginfo.timeStamp);
		std::wstring result = std::format(L"{:%d/%m/%Y %H:%M:%S}", timePoint);

		result += loginfo.content;

		if (loginfo.level >= Level::warning) {
			DWORD lastError = GetLastError();

			LPWSTR messageBuffer = nullptr;

			size_t size = FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				lastError,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				(LPWSTR)&messageBuffer,
				0, nullptr);

			if (size == 0) [[unlikely]] {
				messageBuffer = (wchar_t*)L"Unknown error.";
				size = sizeof(L"Unknown error.") / sizeof(wchar_t);
			}

			result += L" | LastError (" + std::to_wstring(lastError) + L") : " + std::wstring(messageBuffer, size);

			LocalFree(messageBuffer);
		}
		result += L"\n";

		return result;
	}
	constexpr std::wstring LogManager::GetErrorLevel(const Level level) {
		switch (level) {
		case Level::debug:
			return std::wstring(L"DEBUG   ");
		case Level::info:
			return std::wstring(L"INFO    ");
		case Level::warning:
			return std::wstring(L"WARNING ");
		case Level::error:
			return std::wstring(L"ERROR   ");
		case Level::fatal:
			return std::wstring(L"FATAL   ");
		default:
			return std::wstring(L"UNKOWN  ");
		}
	}
	void LogManager::RedirectLog(const Level level, const std::wstring& content) {
		const auto action = actions[level];
		if (action == Action::NONE) return;
		if (action & Action::DEBUG_STRING)	DebugConsol(level, content);
		if (action & Action::MESSAGE_BOX)	MessageBox(level, content);
		if (action & Action::FILE_TEMP)		WriteTruncate(level, content);
		if (action & Action::FILE_PERM)		WriteAppend(level, content);
		if (action & Action::KILL_PROC)		KillProcess(level, content);
	}


	void LogManager::DebugConsolWorker() {
		std::unique_lock<std::mutex> lock(mtxAction[0]);

		while (!shouldStop) {
			actionCVs[0].wait(lock, [] {return wakeFlags[0] || shouldStop; });

			if (shouldStop) break;

			if (wakeFlags[0]) {
				while (!debugConsolLog.empty()) {
					Log log = debugConsolLog.front();
					debugConsolLog.pop();

					OutputDebugStringW(std::wstring(GetErrorLevel(log.level) + log.content).c_str());
				}
				wakeFlags[0] = false;
			}
		}
	}
	void LogManager::DebugConsol(Level level, const std::wstring& content) {
		{
			std::unique_lock<std::mutex> lock(mtxAction[0]);
			debugConsolLog.push({ level, content });
			wakeFlags[0] = true;
		}

		actionCVs[0].notify_one();
	}

	void LogManager::MessageBoxWorker() {
		std::unique_lock<std::mutex> lock(mtxAction[1]);

		while (!shouldStop) {
			actionCVs[1].wait(lock, [] {return wakeFlags[1] || shouldStop; });

			if (shouldStop) break;

			if (wakeFlags[1]) {
				while (!messageBoxLog.empty()) {
					Log log = messageBoxLog.front();
					messageBoxLog.pop();

					UINT type = vbOKOnly;

					if (log.level == Level::debug) type = vbInformation;
					else if (log.level == Level::warning) type = vbExclamation;
					else if (log.level >= Level::error) type = vbCritical;

					std::wstring noTimeStamp = log.content.substr(19);
					MessageBoxW(nullptr, noTimeStamp.c_str(), GetErrorLevel(log.level).c_str(), type);
				}
				wakeFlags[1] = false;
			}
		}
	}
	void LogManager::MessageBox(Level level, const std::wstring& content) {
		{
			std::unique_lock<std::mutex> lock(mtxAction[1]);
			messageBoxLog.push({ level, content });
			wakeFlags[1] = true;
		}

		actionCVs[1].notify_one();
	}

	void LogManager::KillProcessWorker() {
		std::unique_lock<std::mutex> lock(mtxAction[4]);

		while (!shouldStop) {
			actionCVs[4].wait(lock, [] {return wakeFlags[4] || shouldStop; });

			if (shouldStop) break;

			if (wakeFlags[4]) {
				while (!killProcessLog.empty()) {
					Log log = killProcessLog.front();
					killProcessLog.pop();

					std::wstring terminationReason = L"Process termination requested due to " + log.content;
					DebugConsol(Level::fatal, terminationReason);

					MessageBoxW(nullptr, terminationReason.c_str(), L"FATAL - Process Termination", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

					ExitProcess(1);
				}
				wakeFlags[4] = false;
			}
		}
	}
	void LogManager::KillProcess(Level level, const std::wstring& content) {
		{
			std::unique_lock<std::mutex> lock(mtxAction[4]);
			killProcessLog.push({ level, content });
			wakeFlags[4] = true;
		}

		actionCVs[4].notify_one();
	}

	bool LogManager::InitializeFileHandles() {
		constexpr DWORD bufSize = MAX_PATH;
		alignas(16) WCHAR buffer[bufSize];

		DWORD size = GetModuleFileNameW(nullptr, buffer, bufSize);
		if (size == 0) {
			DebugConsol(Level::fatal, L"Couldn't use GetModuleFileNameW, killing process...");
			KillProcess(Level::fatal, L"File initialization failed");
			return false;
		}

		WCHAR* lastSlash = nullptr;
		for (WCHAR* p = buffer; *p; ++p) {
			if (*p == L'\\') lastSlash = p;
		}

		std::wstring dirPath(buffer, lastSlash ? lastSlash - buffer : size);
		dirPath += L"\\Log";

		DWORD attributes = GetFileAttributesW(dirPath.c_str());
		if (!(attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))) {
			if (!CreateDirectoryW(dirPath.c_str(), nullptr)) {
				DebugConsol(Level::fatal, L"Couldn't use CreateDirectoryW, killing process...");
				KillProcess(Level::fatal, L"Log directory creation failed");
				return false;
			}
		}

		// Initialize temp log file
		std::wstring tempLog = dirPath + L"\\tempLog.log";
		hFileTemp = CreateFileW(
			tempLog.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
		if (hFileTemp == INVALID_HANDLE_VALUE) {
			DebugConsol(Level::fatal, L"Couldn't create temp log file, killing process...");
			KillProcess(Level::fatal, L"Temp log file creation failed");
			return false;
		}

		// Initialize permanent log file
		std::wstring permLog = dirPath + L"\\permLog.log";
		attributes = GetFileAttributesW(permLog.c_str());
		if (!(attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY))) {
			hFilePermanent = CreateFileW(
				permLog.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				nullptr
			);
		}
		else {
			hFilePermanent = CreateFileW(
				permLog.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN,
				nullptr
			);

			LARGE_INTEGER fileSizeStruct;
			if (GetFileSizeEx(hFilePermanent, &fileSizeStruct)) {
				offsetPermFile = fileSizeStruct.QuadPart;
			}
		}

		if (hFilePermanent == INVALID_HANDLE_VALUE) {
			DebugConsol(Level::fatal, L"Couldn't create permanent log file, killing process...");
			KillProcess(Level::fatal, L"Permanent log file creation failed");
			return false;
		}

		return true;
	}
	bool LogManager::WriteLog(const HANDLE hFile, UINT64& offset, const std::wstring& content) {
		OVERLAPPED overlapped = { 0 };
		overlapped.Offset = (DWORD)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = (DWORD)(offset >> 32);

		std::vector<UINT8> data;
		std::string logUtf8 = std::string(content.begin(), content.end());
		data.assign(logUtf8.begin(), logUtf8.end());

		DWORD bytesToWrite = static_cast<DWORD>(data.size());
		DWORD bytesWritten = 0;

		bool success = ::WriteFile(
			hFile,
			data.data(),
			bytesToWrite,
			&bytesWritten,
			&overlapped
		);

		if (!success || bytesWritten != bytesToWrite) [[unlikely]] {
			return false;
		}
		else [[likely]] {
			offset += bytesWritten;
			return true;
		}
	}

	void LogManager::WriteTruncateWorker() {
		std::unique_lock<std::mutex> lock(mtxAction[2]);

		while (!shouldStop) {
			actionCVs[2].wait(lock, [] {return wakeFlags[2] || shouldStop; });

			if (shouldStop) break;

			if (wakeFlags[2]) {
				while (!writeTruncateLog.empty()) {
					Log log = writeTruncateLog.front();
					writeTruncateLog.pop();

					std::wstring result = GetErrorLevel(log.level) + log.content;

					if (!WriteLog(hFileTemp, offsetTempFile, result)) [[unlikely]] {
						if (log.level <= Level::warning) {
							MessageBox(log.level, L"Couldn't write the (temporary) log:\n" + result);
						}
						else {
							MessageBox(log.level, L"Couldn't write the (temporary) log:\n" + result + L"\nKilling the process...");
							KillProcess(log.level, L"Temp file write failed");
						}
					}
				}
				wakeFlags[2] = false;
			}
		}
	}
	void LogManager::WriteTruncate(Level level, const std::wstring& content) {
		{
			std::unique_lock<std::mutex> lock(mtxAction[2]);
			writeTruncateLog.push({ level, content });
			wakeFlags[2] = true;
		}

		actionCVs[2].notify_one();
	}

	void LogManager::WriteAppendWorker() {
		std::unique_lock<std::mutex> lock(mtxAction[3]);

		while (!shouldStop) {
			actionCVs[3].wait(lock, [] {return wakeFlags[3] || shouldStop; });

			if (shouldStop) break;

			if (wakeFlags[3]) {
				while (!writeAppendLog.empty()) {
					Log log = writeAppendLog.front();
					writeAppendLog.pop();

					std::wstring result = GetErrorLevel(log.level) + log.content;

					if (!WriteLog(hFilePermanent, offsetPermFile, result)) [[unlikely]] {
						if (log.level <= Level::warning) {
							MessageBox(log.level, L"Couldn't write the (permanent) log:\n" + result);
						}
						else {
							MessageBox(log.level, L"Couldn't write the (permanent) log:\n" + result + L"\nKilling the process...");
							KillProcess(log.level, L"Permanent file write failed");
						}
					}
				}
				wakeFlags[3] = false;
			}
		}
	}
	void LogManager::WriteAppend(Level level, const std::wstring& content) {
		{
			std::unique_lock<std::mutex> lock(mtxAction[3]);
			writeAppendLog.push({ level, content });
			wakeFlags[3] = true;
		}

		actionCVs[3].notify_one();
	}
}
