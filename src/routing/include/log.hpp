#pragma once

#include <format>
#include <string>

namespace Log {

auto configure(std::string_view level, std::string_view file_path) -> void;

void info(std::string_view msg);
void error(std::string_view msg);

template <typename... Args>
inline void info(std::format_string<Args...> fmt, Args &&...args) {
  const auto formatted = std::format(fmt, std::forward<Args>(args)...);
  info(formatted);
}

template <typename... Args>
inline void error(std::format_string<Args...> fmt, Args &&...args) {
  const auto formatted = std::format(fmt, std::forward<Args>(args)...);
  error(formatted);
}

} // namespace Log
