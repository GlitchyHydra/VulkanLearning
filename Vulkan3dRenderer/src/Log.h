#pragma once

#include "memory"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Vipera
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define VIPERA_CORE_TRACE(...)	::Vipera::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define VIPERA_CORE_INFO(...)	::Vipera::Log::GetCoreLogger()->info(__VA_ARGS__);
#define VIPERA_CORE_WARN(...)	::Vipera::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define VIPERA_CORE_ERROR(...)	::Vipera::Log::GetCoreLogger()->error(__VA_ARGS__);
#define VIPERA_CORE_FATAL(...)	::Vipera::Log::GetCoreLogger()->critical(__VA_ARGS__);

//Client Log Macros
#define VIPERA_TRACE(...)	::Vipera::Log::GetClientLogger()->trace(__VA_ARGS__);
#define VIPERA_INFO(...)	::Vipera::Log::GetClientLogger()->info(__VA_ARGS__);
#define VIPERA_WARN(...)	::Vipera::Log::GetClientLogger()->warn(__VA_ARGS__);
#define VIPERA_ERROR(...)	::Vipera::Log::GetClientLogger()->error(__VA_ARGS__);
#define VIPERA_FATAL(...)	::Vipera::Log::GetClientLogger()->critical(__VA_ARGS__);

#ifdef VIPERA_DISTRIBUTION_BUILD 
#define VIPERA_TRACE
#define VIPERA_INFO
#define VIPERA_WARN
#define VIPERA_ERROR
#define VIPERA_FATAL
#endif

