#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Moxel
{
	std::shared_ptr<spdlog::logger> Log::s_logger;

	void Log::initialize()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_logger = spdlog::stdout_color_mt("ENGINE");
		s_logger->set_level(spdlog::level::trace);
	}
}