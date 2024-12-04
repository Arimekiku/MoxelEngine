#pragma once

#include <spdlog/spdlog.h>

#ifdef _WIN32
#define debugbreak() __debugbreak()
#else
#include <signal.h>
#define debugbreak() raise(SIGTRAP)
#endif

namespace SDLarria
{
	class Log
	{
	public:
		static void Initialize();

		static auto& GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}

#define LOG_TRACE(...)		::SDLarria::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::SDLarria::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::SDLarria::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::SDLarria::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)	::SDLarria::Log::GetLogger()->critical(__VA_ARGS__)
