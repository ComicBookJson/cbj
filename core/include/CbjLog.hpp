#pragma once
#include <cstdint>
#include <string>
#include <vector>

/** One buffered log event exposed to native and generated bindings. */
struct CbjLogRecord
{
    /** Numeric log level corresponding to cbj::LogLevel. */
    int level;
    /** Logical component that emitted the message. */
    std::string component;
    /** Human-readable log message. */
    std::string message;
};

namespace cbj
{
    /** Severity levels emitted by the CBJ SDK. */
    enum class LogLevel : int
    {
        /** Extremely detailed diagnostic events. */
        Trace = 0,
        /** Developer-oriented diagnostic events. */
        Debug = 1,
        /** Normal successful operation events. */
        Info = 2,
        /** Significant state transitions that are not errors. */
        Notice = 3,
        /** Recoverable problems or degraded behavior. */
        Warn = 4,
        /** Operation failures and exceptions. */
        Error = 5
    };

    /** Native callback type used by applications that consume the live log stream. */
    using LogCallback = void (*)(LogLevel, const char *, const char *);

    /** Thread-safe logger and buffered log stream facade. */
    class CbjLog
    {
    public:
        /** Installs or clears the native live-log callback. */
        static void SetCallback(LogCallback callback);
        /** Sets the minimum severity retained and emitted. */
        static void SetLevel(LogLevel level);
        /** Returns the current minimum severity. */
        static LogLevel GetLevel();
        /** Returns and clears buffered records for generated bindings. */
        static std::vector<CbjLogRecord> Drain();
        /** Emits a trace message. */
        static void Trace(const std::string &component, const std::string &message);
        /** Emits a debug message. */
        static void Debug(const std::string &component, const std::string &message);
        /** Emits an informational message. */
        static void Info(const std::string &component, const std::string &message);
        /** Emits a notice message. */
        static void Notice(const std::string &component, const std::string &message);
        /** Emits a warning message. */
        static void Warn(const std::string &component, const std::string &message);
        /** Emits an error message. */
        static void Error(const std::string &component, const std::string &message);
    };
}
