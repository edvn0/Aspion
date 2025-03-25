#include "router.hpp"
#include "log.hpp"

namespace Routing {

void register_controller(Router &router, RegistrationFunction func) {
  func(router);
}

auto Router::print_routes() const -> void {
  if (assigned_routes.empty()) {
    Log::info("No registered routes.");
    return;
  }

  auto max_path_width = std::ranges::max(
      assigned_routes | std::views::keys |
      std::views::transform([](const auto &path) { return path.size(); }));

  auto max_controller_width =
      std::ranges::max(route_controllers | std::views::values |
                       std::views::transform([](const auto &controller) {
                         return controller.size();
                       }));

  auto separator =
      std::format("{:-<{}s}", "", max_path_width + max_controller_width + 10);

  Log::info("\n{}", separator);
  Log::info("| {:<{}} | {:<{}} |", "Registered Routes", max_path_width,
            "Controller", max_controller_width);
  Log::info("{}", separator);

  for (const auto &path : assigned_routes | std::views::keys) {
    const auto &controller = route_controllers.at(path);
    Log::info("| {:<{}} | {:<{}} |", path, max_path_width, controller,
              max_controller_width);
  }

  Log::info("{}", separator);
}

} // namespace Routing
