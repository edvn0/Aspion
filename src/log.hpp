#pragma once

#include <format>
#include <string>

namespace Log {

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
