#pragma once

#include <boost/json.hpp>

namespace Routing {

template <typename Dict> auto json(const Dict &dict) -> boost::json::object {
  boost::json::object obj;
  for (auto &&[k, v] : dict) {
    obj[k] = v;
  }
  return obj;
}

template <typename Key = std::string, typename Value = boost::json::value>
auto json(const std::initializer_list<std::pair<Key, Value>> &dict)
    -> boost::json::object {
  boost::json::object obj;
  for (auto &&[k, v] : dict) {
    obj[k] = v;
  }
  return obj;
}

} // namespace Routing
