#include "log.hpp"

#include <mutex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Log {

namespace {
static std::string log_level{};
static std::string log_file_path{};

auto init_logger() -> std::shared_ptr<spdlog::logger> {
  auto spd_log_level = spdlog::level::from_str(log_level);

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spd_log_level);
  console_sink->set_pattern("[Aspion %Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);
  file_sink->set_level(spd_log_level);
  file_sink->set_pattern("[Aspion %Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

  auto logger = std::make_shared<spdlog::logger>(
      "aspion", spdlog::sinks_init_list{console_sink, file_sink});
  logger->set_level(spd_log_level);
  spdlog::set_default_logger(logger);
  spdlog::flush_on(spdlog::level::err);
  spdlog::flush_every(std::chrono::seconds(7));

  return logger;
}

auto get_logger() -> std::shared_ptr<spdlog::logger> {
  static std::shared_ptr<spdlog::logger> logger = init_logger();
  return logger;
}

} // namespace

auto configure(std::string_view level, std::string_view file_path) -> void {
  log_level = level;
  log_file_path = file_path;
}

void info(std::string_view msg) { get_logger()->info("{}", msg); }

void error(std::string_view msg) { get_logger()->error("{}", msg); }

} // namespace Log
