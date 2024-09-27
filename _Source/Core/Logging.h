#pragma once

#include <format>
#include <string>
#include <vector>
#include <iostream>
#include <ranges>
#include <chrono>

#if _WIN32
#define NOMINMAX
#include <windows.h>
#undef min
#undef max
#endif

#ifdef _DEBUG
#define ASSERT(condition, message)                                           \
		if (!(condition))                                                        \
		{                                                                        \
			std::cerr << "Assertion `" #condition "` failed in " << __FILE__     \
					  << " line " << __LINE__ << ": " << (message) << std::endl; \
			std::terminate();                                                    \
		}
#else
#define ASSERT(x, y)
#endif

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

		Logger(Logger& other)               = delete;
		~Logger()                           = default;
		void operator=(const Logger& other) = delete;

		static Logger* getInstance();

		template <typename... Types>
		void log(std::format_string<Types...> fmt, ELogLevel inLevel, Types&&... args)
		{
			std::string msg = std::format(fmt, std::forward<Types>(args)...);
			std::string outMsg;
			auto now           = std::chrono::system_clock::now();
			const auto fmtTime = std::format("{0:%F %T}", now);
			outMsg += "[" + fmtTime + "] ";
			switch (inLevel)
			{
			case ELogLevel::Debug:
				{
					outMsg += "[DEBUG] " + msg + '\n';
					break;
				}
			case ELogLevel::Info:
				{
					outMsg += "[INFO] " + msg + '\n';
					break;
				}
			case ELogLevel::Warning:
				{
					outMsg += "[WARNING] " + msg + '\n';
					break;
				}
			case ELogLevel::Error:
				{
					outMsg += "[ERROR] " + msg + '\n';
					break;
				}
			}

			if (inLevel != ELogLevel::Debug)
			{
				m_messages.emplace_back(outMsg, inLevel);
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

		void clear()
		{
			m_messages.clear();
		}
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

#define LOG_DEBUG(x, ...) Logging::debug(x, __VA_ARGS__);
#define LOG_INFO(x, ...) Logging::info(x, __VA_ARGS__);
#define LOG_WARNING(x, ...) Logging::warning(x, __VA_ARGS__);
#define LOG_ERROR(x, ...) Logging ::error(x, __VA_ARGS__);
