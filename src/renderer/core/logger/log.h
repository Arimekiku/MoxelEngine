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
		static void initialize();

		static auto& get_logger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}

#define LOG_TRACE(...)		::Moxel::Log::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::Moxel::Log::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::Moxel::Log::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::Moxel::Log::get_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)	::Moxel::Log::get_logger()->critical(__VA_ARGS__)
#define LOG_ASSERT(x, ...)	{ if (x == false) { LOG_ERROR("Invalid assertion: {0}", __VA_ARGS__); debugbreak(); } }
