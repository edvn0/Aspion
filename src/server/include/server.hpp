#pragma once

#include "router.hpp"

#include <functional>

namespace Aspion::Server {
struct CLIOptions;
auto start(int argc, char **argv,
           const std::function<void(Routing::Router &)> &route_builder) -> int;
} // namespace Aspion::Server
