#pragma once

#include <cstdint>
#include <string>

#include <boost/functional/hash.hpp>

namespace Util {

template <typename V> struct TransparentHash {
  using is_transparent = void;

  auto operator()(const V &v) const -> std::size_t {
    static constexpr boost::hash<V> hasher;
    return hasher(v);
  }
};

auto generate_uuid() -> std::string;

} // namespace Util