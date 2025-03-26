#pragma once

#include <boost/json.hpp>

#include "util.hpp"

namespace Routing {

using JSONInput = std::unordered_map<std::string_view, boost::json::value,
                                     Util::TransparentHash<std::string_view>,
                                     std::equal_to<>>;

auto json(JSONInput &&dict) -> std::string {
  boost::json::object obj;
  for (auto &&[key, value] : dict) {
    obj[std::string(key)] = value;
  }
  return boost::json::serialize(obj);
}

} // namespace Routing
