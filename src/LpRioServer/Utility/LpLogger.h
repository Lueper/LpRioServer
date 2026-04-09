#pragma once

#include "pch.h"

enum class ELogType : uint8_t {
	debug,
	info,
	warn,
	error,
	fatal
};

const std::string LOG_DESC[]{ "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

using namespace Concurrency;

#define LOG_DEBUG(msg) LOG(ELogType::debug, msg)
#define LOG_INFO(msg) LOG(ELogType::info, msg)
#define LOG_WARN(msg) LOG(ELogType::warn, msg)
#define LOG_ERROR(msg) LOG(ELogType::error, msg)
#define LOG_FATAL(msg) LOG(ELogType::fatal, msg)

class LpLogger {
public:
	static void LOG(ELogType logType, const std::string& msg);
	static void Update();

private:
	static void PushLog(ELogType logType, const std::string& str);
	static void Print(ELogType logType, const std::string& str);
	static void SetColor(ELogType logType);
	static void ResetColor();

	static concurrent_queue<std::pair<ELogType, std::string>> m_logQueue;
	static std::mutex m_mutex;
};