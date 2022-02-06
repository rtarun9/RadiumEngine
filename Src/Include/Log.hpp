#pragma once

#include <iostream>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define SPDLOG_DEBUG_ON
#define SPDLOG_TRACE_ON

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace rad
{
	class Log
	{
	public:
		inline void Init()
		{
			// Debug to console in Debug mode.
#ifdef _DEBUG
			AllocConsole();
			freopen("CON", "w", stdout);
#endif
			spdlog::set_pattern("%^[%T] %n : %v%$");
			s_CoreLogger = spdlog::stdout_color_mt("RADIUM");
			s_CoreLogger->set_level(spdlog::level::trace);
		}

		static inline std::shared_ptr<spdlog::logger>& GetCoreLogger()
		{
			return s_CoreLogger;
		}
	
	private:
		static inline std::shared_ptr<spdlog::logger> s_CoreLogger;
	};
}

#ifdef _DEBUG
	#define RAD_CORE_ERROR(...) ::rad::Log::GetCoreLogger()->error(__VA_ARGS__) 
	#define RAD_CORE_INFO(...) ::rad::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define RAD_CORE_WARN(...) ::rad::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define RAD_CORE_TRACE(...) ::rad::Log::GetCoreLogger()->trace(__VA_ARGS__)
#else
	#define RAD_CORE_ERROR(...)
	#define RAD_CORE_INFO(...) 
	#define RAD_CORE_WARN(...) 
	#define RAD_CORE_TRACE(...)
#endif