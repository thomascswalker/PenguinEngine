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

#define LOG_DEBUG(X, ...) Logging::debug(X, __VA_ARGS__);
#define LOG_INFO(X, ...) Logging::info(X, __VA_ARGS__);
#define LOG_WARNING(X, ...) Logging::warning(X, __VA_ARGS__);
#define LOG_ERROR(X, ...) Logging ::error(X, __VA_ARGS__);

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
		std::vector<std::pair<std::string, ELogLevel>> m_messages;

		Logger() = default;

	public:
		static Logger* m_instance;
		static int m_line;
		static int m_column;
		static std::string m_source;

		Logger(Logger& other) = delete;
		~Logger() = default;
		void operator=(const Logger& other) = delete;

		static Logger* getInstance();

		template <typename... Types>
		void log(std::format_string<Types...> fmt, ELogLevel inLevel, Types&&... args)
		{
			std::string msg = std::format(fmt, std::forward<Types>(args)...);
			m_messages.push_back({msg, inLevel});

			std::string outMsg;
			auto now = std::chrono::system_clock::now();
			const auto fmtTime = std::format("{0:%F %T}", now);
			outMsg += "[" + fmtTime + "] ";
			switch (inLevel)
			{
			case ELogLevel::Debug:
				outMsg += "[DEBUG] " + msg + '\n';
				break;
			case ELogLevel::Info:
				outMsg += "[INFO] " + msg + '\n';
				break;
			case ELogLevel::Warning:
				outMsg += "[WARNING] " + msg + '\n';
				break;
			case ELogLevel::Error:
				outMsg += "[ERROR] " + msg + '\n';
				break;
			}
#if _WIN32
			// Windows apps which launch through the wWinMain do not output to
			// the console. This handles that.
			const auto wStrMsg = std::wstring(outMsg.begin(), outMsg.end());
			OutputDebugStringW(wStrMsg.c_str());
#else
            // Otherwise just use standard out.
            std::cout << OutMsg << '\n';
#endif
		}

		int getCount(ELogLevel inLevel);
		std::vector<std::string> getMessages(ELogLevel inLevel);

		void clear() { m_messages.clear(); }
	};

	template <typename... Types>
	static constexpr void debug(std::format_string<Types...> fmt, Types&&... args)
	{
		std::cout << std::format(fmt, std::forward<Types>(args)...) << std::endl;
		Logger::getInstance()->log(fmt, ELogLevel::Debug, std::forward<Types>(args)...);
	}

	template <typename... Types>
	static constexpr void info(std::format_string<Types...> fmt, Types&&... args)
	{
		Logger::getInstance()->log(fmt, ELogLevel::Info, std::forward<Types>(args)...);
	}

	template <typename... Types>
	static constexpr void warning(std::format_string<Types...> fmt, Types&&... args)
	{
		Logger::getInstance()->log(fmt, ELogLevel::Warning, std::forward<Types>(args)...);
	}

	template <typename... Types>
	static constexpr void error(std::format_string<Types...> fmt, Types&&... args)
	{
		Logger::getInstance()->log(fmt, ELogLevel::Error, std::forward<Types>(args)...);
	}
} // namespace Logging
