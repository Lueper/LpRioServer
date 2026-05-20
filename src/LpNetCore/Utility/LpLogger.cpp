#include "LpLogger.h"

std::mutex LpLogger::m_mutex;
concurrent_queue<std::pair<ELogType, std::string>> LpLogger::m_logQueue;

void LpLogger::Update() {
	std::pair<ELogType, std::string> log;

	while (m_logQueue.try_pop(log)) {
		Print(log.first, log.second);
	}
}

void LpLogger::FormatLog(std::ostringstream& os, ELogType logType) {
	struct _timeb _time;
	tm t;

	_ftime64_s(&_time);
	localtime_s(&t, &(_time.time));

	// [yyyy-mm-dd hh:mm:ss.ms]
	os << t.tm_year + 1900 << "-"
	   << std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "-"
	   << std::setfill('0') << std::setw(2) << t.tm_mday << " "
	   << std::setfill('0') << std::setw(2) << t.tm_hour << ":"
	   << std::setfill('0') << std::setw(2) << t.tm_min << ":"
	   << std::setfill('0') << std::setw(2) << t.tm_sec << "."
	   << std::setfill('0') << std::setw(3) << _time.millitm << " ";
	// LogType
	os << std::setfill(' ') << std::setw(6) << std::left << LOG_DESC[(int)logType] << "[";
	// ThreadID
	os << std::this_thread::get_id() << "] ";
}

void LpLogger::PushLog(ELogType logType, const std::string& str) {
	m_logQueue.push(std::make_pair(logType, str));
}

void LpLogger::Print(ELogType logType, const std::string& str) {
	std::lock_guard<std::mutex> lock(m_mutex);

	SetColor(logType);

	if (!WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str.c_str(), static_cast<DWORD>(str.size()), NULL, NULL))
		std::cerr << "Failed to write to console!" << std::endl;

	ResetColor();
}

void LpLogger::SetColor(ELogType logType) {
	switch (logType) {
		case ELogType::debug:  // cyan
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
									FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
		case ELogType::info:  // white
			break;
		case ELogType::warn:  // yellow
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
									FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			break;
		case ELogType::error:  // red
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
			break;
		case ELogType::fatal:  // background red
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN |
																		 FOREGROUND_BLUE | FOREGROUND_INTENSITY |
																		 BACKGROUND_RED);
			break;
		default:
			break;
	}
}

void LpLogger::ResetColor() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}