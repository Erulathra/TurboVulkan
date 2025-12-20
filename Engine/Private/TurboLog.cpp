#include "TurboLog.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

entt::dense_map<entt::hashed_string::value_type, Turbo::LogVerbosity> Turbo::gLogVerbosityMap;

constexpr cstring kLogFilePath = "Saved/Logs/TurboEngine.log";
constexpr cstring kBackupFileLogPath = "Saved/Logs/TurboEngine.bc.log";

void Turbo::InitLogger()
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

	spdlog::set_level(spdlog::level::debug);

	spdlog::init_thread_pool(8192, 1);
	auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Saved/Logs/TurboEngine.log");

	std::vector<spdlog::sink_ptr> sinks {stdoutSink, fileSink};
	auto logger = std::make_shared<spdlog::async_logger>("defaultLogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	spdlog::register_logger(logger);
	spdlog::set_default_logger(logger);

	spdlog::flush_every(std::chrono::nanoseconds(250));

	TURBO_LOG(LogEngine, Info, "Logger settings initialized.")
}
