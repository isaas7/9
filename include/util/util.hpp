#ifndef UTIL_HPP
#define UTIL_HPP
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
std::shared_ptr<spdlog::logger> initialize_logger();
#endif
