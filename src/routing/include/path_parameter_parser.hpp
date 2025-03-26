#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace Routing::Util {

template <typename T> struct PathParameterParser {
  auto parse(const std::string_view value) const -> std::optional<T> = delete;
};

template <std::integral T> struct PathParameterParser<T> {
  using Size = sizeof(T);
  static_assert(Size == 1 || Size == 2 || Size == 4 || Size == 8,
                "Unsupported integral type");
  auto parse(const std::string_view value) const -> std::optional<T> {
    T result{};
    auto [ptr, ec] = std::from_chars(value.data(), value.data() + Size, result);
    if (ec == std::errc{})
      return result;
    return std::nullopt;
  }
};

template <std::floating_point T> struct PathParameterParser<T> {
  using Size = sizeof(T);
  static_assert(Size == 4 || Size == 8, "Unsupported floating point type");
  auto parse(const std::string_view value) const -> std::optional<T> {
    T result{};
    auto [ptr, ec] = std::from_chars(value.data(), value.data() + Size, result);
    if (ec == std::errc{})
      return result;
    return std::nullopt;
  }
};

template <> struct PathParameterParser<bool> {
  auto parse(const std::string_view value) const -> std::optional<bool> {
    if (value == "true" || value == "1")
      return true;
    if (value == "false" || value == "0")
      return false;
    return std::nullopt;
  }
};

template <> struct PathParameterParser<std::string> {
  auto parse(const std::string_view value) const -> std::optional<std::string> {
    return std::string(value);
  }
};

template <> struct PathParameterParser<std::string_view> {
  auto parse(const std::string_view value) const
      -> std::optional<std::string_view> {
    return value;
  }
};

} // namespace Routing::Util
