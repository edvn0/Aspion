#pragma once

namespace Util {

template <typename V> struct TransparentHash {
  using is_transparent = void;

  auto operator()(const V &v) const -> std::size_t {
    static constexpr boost::hash<V> hasher;
    return hasher(v);
  }
};

} // namespace Util