#pragma once

#include <concurrent_queue.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/timeb.h>
#include <windows.h>

enum class ELogType : uint8_t {
	debug,
	info,
	warn,
	error,
	fatal
};

const std::string LOG_DESC[]{ "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

using namespace Concurrency;

#define LOG_DEBUG(...)		LpLogger::Log(ELogType::debug, __VA_ARGS__)
#define LOG_INFO(...)		LpLogger::Log(ELogType::info, __VA_ARGS__)
#define LOG_WARN(...)		LpLogger::Log(ELogType::warn, __VA_ARGS__)
#define LOG_ERROR(...)		LpLogger::Log(ELogType::error, __VA_ARGS__)
#define LOG_FATAL(...)		LpLogger::Log(ELogType::fatal, __VA_ARGS__)

class LpLogger {
public:
	template <typename... Args>
	static void Log(ELogType logType, Args... args) {
		std::ostringstream os;

		FormatLog(os, logType);
		MergeArgs(os, args...);
		os << "\n";

		Write(logType, os.str());
	}

	static void Update();

private:
	template <typename... Args>
	static void MergeArgs(std::ostringstream& os, Args... args) {
		int merge[] = { 0, (os << args, 0)... };
		static_cast<void>(merge);
	}

	static void Write(ELogType logType, const std::string& str) {
#ifdef USE_ASYNC_LOG
		PushLog(logType, str);
#else
		Print(logType, str);
#endif
	}

	static void FormatLog(std::ostringstream& os, ELogType logType);
	static void PushLog(ELogType logType, const std::string& str);
	static void Print(ELogType logType, const std::string& str);
	static void SetColor(ELogType logType);
	static void ResetColor();

	static concurrent_queue<std::pair<ELogType, std::string>> m_logQueue;
	static std::mutex m_mutex;
};