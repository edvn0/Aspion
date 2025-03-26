#pragma once

#include "request_response.hpp"

namespace Routing {

using Middleware = std::function<Core::Response(const Core::Request &)>;

class IMiddleware {
public:
  virtual ~IMiddleware() = default;

  virtual auto invoke(const Core::Request &, const Middleware &)
      -> Core::Response = 0;
};

} // namespace Routing
