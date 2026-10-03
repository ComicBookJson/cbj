#define LOGFAULT_ENABLE_DEFAULT_LOGGER 0
#include "CbjLog.hpp"
#include "logfault/logfault.h"
#include <mutex>
#include <iostream>
#include <vector>

namespace cbj {
namespace {
std::mutex g_mutex;
LogCallback g_callback = nullptr;
LogLevel g_level = LogLevel::Info;
std::vector<CbjLogRecord> g_records;
void emit(LogLevel level, const std::string& component, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (static_cast<int>(level) < static_cast<int>(g_level)) return;
    g_records.push_back({static_cast<int>(level), component, message});
    if (g_records.size() > 1024) g_records.erase(g_records.begin(), g_records.begin() + 256);
    if (g_callback) g_callback(level, component.c_str(), message.c_str());
    switch (level) {
        case LogLevel::Trace: LFLOG_TRACE << "[" << component << "] " << message; break;
        case LogLevel::Debug: LFLOG_DEBUG << "[" << component << "] " << message; break;
        case LogLevel::Info: LFLOG_INFO << "[" << component << "] " << message; break;
        case LogLevel::Notice: LFLOG_NOTICE << "[" << component << "] " << message; break;
        case LogLevel::Warn: LFLOG_WARN << "[" << component << "] " << message; break;
        case LogLevel::Error: LFLOG_ERROR << "[" << component << "] " << message; break;
    }
}
}
void CbjLog::SetCallback(LogCallback cb){ std::lock_guard<std::mutex> l(g_mutex); g_callback=cb; }
void CbjLog::SetLevel(LogLevel l){ std::lock_guard<std::mutex> x(g_mutex); g_level=l; }
LogLevel CbjLog::GetLevel(){ std::lock_guard<std::mutex> x(g_mutex); return g_level; }
std::vector<CbjLogRecord> CbjLog::Drain(){ std::lock_guard<std::mutex> x(g_mutex); auto r=g_records; g_records.clear(); return r; }
void CbjLog::Trace(const std::string& c,const std::string& m){emit(LogLevel::Trace,c,m);}
void CbjLog::Debug(const std::string& c,const std::string& m){emit(LogLevel::Debug,c,m);}
void CbjLog::Info(const std::string& c,const std::string& m){emit(LogLevel::Info,c,m);}
void CbjLog::Notice(const std::string& c,const std::string& m){emit(LogLevel::Notice,c,m);}
void CbjLog::Warn(const std::string& c,const std::string& m){emit(LogLevel::Warn,c,m);}
void CbjLog::Error(const std::string& c,const std::string& m){emit(LogLevel::Error,c,m);}
}