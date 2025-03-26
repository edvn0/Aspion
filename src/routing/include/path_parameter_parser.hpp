#pragma once

#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace Routing::Util {

template <typename Derived, typename T> struct base_path_parameter_parser {
  auto parse(const std::string_view value) const -> std::optional<T> {
    if (value.empty())
      return std::nullopt;
    return static_cast<const Derived *>(this)->parse_impl(value);
  }

protected:
  base_path_parameter_parser() = default;
  ~base_path_parameter_parser() = default;
};

template <std::integral T>
struct integral_path_parameter_parser
    : base_path_parameter_parser<integral_path_parameter_parser<T>, T> {
  auto parse_impl(const std::string_view value) const -> std::optional<T> {
    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc{})
      return result;
    return std::nullopt;
  }
};

template <std::floating_point T>
struct floating_path_parameter_parser
    : base_path_parameter_parser<floating_path_parameter_parser<T>, T> {
  auto parse_impl(const std::string_view value) const -> std::optional<T> {
    T result{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), result);
    if (ec == std::errc{})
      return result;
    return std::nullopt;
  }
};

struct bool_path_parameter_parser
    : base_path_parameter_parser<bool_path_parameter_parser, bool> {
  auto parse_impl(const std::string_view value) const -> std::optional<bool> {
    if (value == "true" || value == "1")
      return true;
    if (value == "false" || value == "0")
      return false;
    return std::nullopt;
  }
};

struct string_path_parameter_parser
    : base_path_parameter_parser<string_path_parameter_parser, std::string> {
  auto parse_impl(const std::string_view value) const
      -> std::optional<std::string> {
    return std::string(value);
  }
};

struct string_view_path_parameter_parser
    : base_path_parameter_parser<string_view_path_parameter_parser,
                                 std::string_view> {
  auto parse_impl(const std::string_view value) const
      -> std::optional<std::string_view> {
    return value;
  }
};

template <typename T> struct PathParameterParser;

template <std::integral T>
struct PathParameterParser<T> : integral_path_parameter_parser<T> {};

template <std::floating_point T>
struct PathParameterParser<T> : floating_path_parameter_parser<T> {};

template <> struct PathParameterParser<bool> : bool_path_parameter_parser {};

template <>
struct PathParameterParser<std::string> : string_path_parameter_parser {};

template <>
struct PathParameterParser<std::string_view>
    : string_view_path_parameter_parser {};

} // namespace Routing::Util
