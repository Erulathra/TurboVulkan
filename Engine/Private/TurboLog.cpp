#include "TurboLog.h"

#include "Core/FileSystem.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

namespace Turbo
{
	entt::dense_map<entt::hashed_string::value_type, LogVerbosity> gLogVerbosityMap;

	inline const std::string kLogFilePath = FileSystem::PathCombine(FileSystem::kLogPath, "TurboEngine.log");
	inline const std::string kBackupFileLogPath = FileSystem::PathCombine(FileSystem::kLogPath, "TurboEngine.bc.log");

	void InitLogger()
	{
		if (std::filesystem::exists(kLogFilePath))
		{
			if (std::filesystem::exists(kBackupFileLogPath))
			{
				std::filesystem::remove(kBackupFileLogPath);
			}

			std::filesystem::copy_file(kLogFilePath, kBackupFileLogPath);
			std::filesystem::remove(kLogFilePath);
		}

		spdlog::init_thread_pool(8192, 1);
		auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Saved/Logs/TurboEngine.log");

		std::vector<spdlog::sink_ptr> sinks {stdoutSink, fileSink};
		auto logger = std::make_shared<spdlog::async_logger>("defaultLogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		spdlog::register_logger(logger);
		spdlog::set_default_logger(logger);

		spdlog::set_level(spdlog::level::trace);

		spdlog::flush_every(std::chrono::nanoseconds(250));

		TURBO_LOG(LogEngine, Info, "Logger settings initialized.")
	}
}

