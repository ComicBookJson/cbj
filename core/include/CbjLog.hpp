#pragma once
#include <cstdint>
#include <string>
#include <vector>
struct CbjLogRecord { int level; std::string component; std::string message; };

namespace cbj {
enum class LogLevel : int { Trace=0, Debug=1, Info=2, Notice=3, Warn=4, Error=5 };
using LogCallback = void (*)(LogLevel, const char*, const char*);

class CbjLog {
public:
    static void SetCallback(LogCallback callback);
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();
    static std::vector<CbjLogRecord> Drain();
    static void Trace(const std::string& component, const std::string& message);
    static void Debug(const std::string& component, const std::string& message);
    static void Info(const std::string& component, const std::string& message);
    static void Notice(const std::string& component, const std::string& message);
    static void Warn(const std::string& component, const std::string& message);
    static void Error(const std::string& component, const std::string& message);
};
}