#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <array>
#include <string>
#include <queue>
#include <chrono>
#include <Windows.h>

#ifdef MessageBox
#undef MessageBox
#endif

namespace LogManager {
class LogManager {
public:
	enum Level {
		debug,
		info,
		warning,
		error,
		fatal
	};
	enum Action {
		NONE = 0,
		DEBUG_STRING = 1 << 0,
		MESSAGE_BOX = 1 << 1,
		FILE_TEMP = 1 << 2,
		FILE_PERM = 1 << 3,
		KILL_PROC = 1 << 4,
	};

	static void SetAction(const Level level, const Action action) {
		actions[level] = action;
	}

	static void LOG(const Level level, const std::wstring source, const std::wstring message);
	static void LOG(const Level level, const std::string source, const std::string message);

private:
	struct LogInfo {
		const Level level;
		const std::chrono::system_clock::time_point timeStamp;
		const unsigned long lastError;
		const std::wstring content;
	};
	struct Log {
		const Level level;
		const std::wstring content;
	};

	static std::array<Action, 5> actions;

	static void MainLogWorker();

	static bool initialized;
	static bool InitalizeAll();
	static void LogCleanUp();

	static void DebugConsolWorker();
	static void MessageBoxWorker();
	static void WriteTruncateWorker();
	static void WriteAppendWorker();
	static void KillProcessWorker();

	static void DebugConsol(Level level, const std::wstring& content);
	static void MessageBox(Level level, const std::wstring& content);
	static void WriteTruncate(Level level, const std::wstring& content);
	static void WriteAppend(Level level, const std::wstring& content);
	static void KillProcess(Level level, const std::wstring& content);


	static std::array<std::condition_variable, 5> actionCVs;
	static std::array<bool, 5> wakeFlags;
	static std::array<std::thread, 5> actionThreads;
	static std::array<std::mutex, 5> mtxAction;

	static std::condition_variable logCV;
	static bool logWakeFlag;
	static std::thread logThread;
	static std::mutex mtx;

	static bool shouldStop;

	static std::queue<LogInfo> logQueue;

	static void RedirectLog(const Level level, const std::wstring& content);
	static std::wstring FormatLog(const LogInfo& loginfo);
	static constexpr std::wstring GetErrorLevel(const Level level);

	static std::queue<Log> debugConsolLog;
	static std::queue<Log> messageBoxLog;
	static std::queue<Log> writeTruncateLog;
	static std::queue<Log> writeAppendLog;
	static std::queue<Log> killProcessLog;

	struct FileHandle {
		HANDLE handle = INVALID_HANDLE_VALUE;

		FileHandle() = default;
		explicit FileHandle(HANDLE h) : handle(h) {}

		~FileHandle() {
			Close();
		}
		FileHandle(const FileHandle&) = delete;
		FileHandle& operator=(const FileHandle&) = delete;
		FileHandle(FileHandle&& other) noexcept : handle(other.handle) {
			other.handle = INVALID_HANDLE_VALUE;
		}

		FileHandle& operator=(FileHandle&& other) noexcept {
			if (this != &other) {
				Close();
				handle = other.handle;
				other.handle = INVALID_HANDLE_VALUE;
			}
			return *this;
		}
		FileHandle& operator=(HANDLE newHandle) {
			if (handle != newHandle) {
				Close();
				handle = newHandle;
			}
			return *this;
		}
		operator HANDLE() const { return handle; }

		void Close() {
			if (handle != INVALID_HANDLE_VALUE) {
				CloseHandle(handle);
				handle = INVALID_HANDLE_VALUE;
			}
		}
	};

	static FileHandle hFileTemp;
	static FileHandle hFilePermanent;
	static UINT64 offsetTempFile;
	static UINT64 offsetPermFile;

	static bool InitializeFileHandles();
	static bool WriteLog(const HANDLE hFile, UINT64& offset, const std::wstring& content);
};
}

#define LOG_DEBUG(source, message) LogManager::LogManager::LOG(LogManager::LogManager::Level::debug, source, message);
#define LOG_INFO(source, message) LogManager::LogManager::LOG(LogManager::LogManager::Level::info, source, message);
#define LOG_WARNING(source, message) LogManager::LogManager::LOG(LogManager::LogManager::Level::warning, source, message);
#define LOG_ERROR(source, message) LogManager::LogManager::LOG(LogManager::LogManager::Level::error, source, message);
#define LOG_FATAL(source, message) LogManager::LogManager::LOG(LogManager::LogManager::Level::fatal, source, message);
