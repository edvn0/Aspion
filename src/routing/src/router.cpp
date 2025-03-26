#include "router.hpp"
#include "log.hpp"

namespace Routing {

void register_controller(Router &router, RegistrationFunction func) {
  func(router);
}

auto Router::print_routes() const -> void {
  // EMPTY
}

} // namespace Routing
