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

#ifdef USE_ASYNC_LOG
#define LOG_DEBUG(msg) LpLogger::LogAsync(ELogType::debug, msg)
#define LOG_INFO(msg) LpLogger::LogAsync(ELogType::info, msg)
#define LOG_WARN(msg) LpLogger::LogAsync(ELogType::warn, msg)
#define LOG_ERROR(msg) LpLogger::LogAsync(ELogType::error, msg)
#define LOG_FATAL(msg) LpLogger::LogAsync(ELogType::fatal, msg)
#else
#define LOG_DEBUG(msg) LpLogger::Log(ELogType::debug, msg)
#define LOG_INFO(msg) LpLogger::Log(ELogType::info, msg)
#define LOG_WARN(msg) LpLogger::Log(ELogType::warn, msg)
#define LOG_ERROR(msg) LpLogger::Log(ELogType::error, msg)
#define LOG_FATAL(msg) LpLogger::Log(ELogType::fatal, msg)
#endif

class LpLogger {
public:
	static void Log(ELogType logType, const std::string& msg);
	static void LogAsync(ELogType logType, const std::string& msg);
	static void Update();

private:
	static std::string FormatLog(ELogType logType, const std::string& str);
	static void PushLog(ELogType logType, const std::string& str);
	static void Print(ELogType logType, const std::string& str);
	static void SetColor(ELogType logType);
	static void ResetColor();

	static concurrent_queue<std::pair<ELogType, std::string>> m_logQueue;
	static std::mutex m_mutex;
};