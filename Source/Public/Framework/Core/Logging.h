#pragma once

#include <format>
#include <string>
#include <vector>
#include <iostream>
#include <ranges>
#include <chrono>

#if _WIN32
#include <windows.h>
#endif

#define LOG_DEBUG(X, ...) Logging::Debug(X, __VA_ARGS__);
#define LOG_INFO(X, ...) Logging::Info(X, __VA_ARGS__);
#define LOG_WARNING(X, ...) Logging::Warning(X, __VA_ARGS__);
#define LOG_ERROR(X, ...) Logging ::Error(X, __VA_ARGS__);

namespace Logging
{
    enum class ELogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
    };

    class Logger
    {
        std::vector<std::pair<std::string, ELogLevel>> Messages;

        Logger() = default;

    public:
        static Logger* Instance;
        static int Line;
        static int Column;
        static std::string Source;

        Logger(Logger& Other) = delete;
        ~Logger() = default;
        void operator=(const Logger& Other) = delete;

        static Logger* GetInstance();

        template <typename... Types>
        void Log(std::format_string<Types...> Fmt, ELogLevel InLevel, Types&&... Args)
        {
            std::string Msg = std::format(Fmt, std::forward<Types>(Args)...);
            Messages.push_back({Msg, InLevel});

            std::string OutMsg;
            auto Now = std::chrono::system_clock::now();
            const auto FmtTime = std::format("{0:%F %T}", Now);
            OutMsg += "[" + FmtTime + "] ";
            switch (InLevel)
            {
            case ELogLevel::Debug :
                OutMsg += "[DEBUG] " + Msg + '\n';
                break;
            case ELogLevel::Info :
                OutMsg += "[INFO] " + Msg + '\n';
                break;
            case ELogLevel::Warning :
                OutMsg += "[WARNING] " + Msg + '\n';
                break;
            case ELogLevel::Error :
                OutMsg += "[ERROR] " + Msg + '\n';
                break;
            }
#if _WIN32
            // Windows apps which launch through the wWinMain do not output to
            // the console. This handles that.
            const auto WStrMsg = std::wstring(OutMsg.begin(), OutMsg.end());
            OutputDebugStringW(WStrMsg.c_str());
#else
            // Otherwise just use standard out.
            std::cout << OutMsg << '\n';
#endif
        }

        int GetCount(ELogLevel InLevel);
        std::vector<std::string> GetMessages(ELogLevel InLevel);

        void Clear() { Messages.clear(); }
    };

    template <typename... Types>
    static constexpr void Debug(std::format_string<Types...> Fmt, Types&&... Args)
    {
        std::cout << std::format(Fmt, std::forward<Types>(Args)...) << std::endl;
        Logger::GetInstance()->Log(Fmt, ELogLevel::Debug, std::forward<Types>(Args)...);
    }

    template <typename... Types>
    static constexpr void Info(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, ELogLevel::Info, std::forward<Types>(Args)...);
    }

    template <typename... Types>
    static constexpr void Warning(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, ELogLevel::Warning, std::forward<Types>(Args)...);
    }

    template <typename... Types>
    static constexpr void Error(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, ELogLevel::Error, std::forward<Types>(Args)...);
    }
} // namespace Logging
