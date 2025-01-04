#pragma once

#include <spdlog/spdlog.h>

#ifdef _WIN32
#define debugbreak() __debugbreak()
#else
#include <signal.h>
#define debugbreak() raise(SIGTRAP)
#endif

namespace Moxel
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

#define LOG_TRACE(...)		::Moxel::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::Moxel::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::Moxel::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::Moxel::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)	::Moxel::Log::GetLogger()->critical(__VA_ARGS__)
#define LOG_ASSERT(x, ...)	{ if (x == false) { LOG_ERROR("Invalid assertion: {0}", __VA_ARGS__); debugbreak(); } }
