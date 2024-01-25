#include "../include/util.hpp"

std::shared_ptr<spdlog::logger> initialize_logger() {
  auto logger = spdlog::stdout_color_mt("console_logger");
  logger->set_level(spdlog::level::trace);
  spdlog::set_default_logger(logger);
  spdlog::flush_every(std::chrono::seconds(3));
  return logger;
}
auto console_logger = initialize_logger();
