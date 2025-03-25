#pragma once

#include "router.hpp"

#include <functional>

namespace Aspion::Server {
struct CLIOptions;
auto main(int argc, char **argv,
          std::function<void(Routing::Router &)> route_builder) -> int;
} // namespace Aspion::Server
