#include "router.hpp"

namespace Routing {

void register_controller(Router &router, RegistrationFunction func) {
  func(router);
}

auto Router::print_routes(std::ostream &out) const->void {
  if (assigned_routes.empty()) {
    out << "No registered routes.\n";
    return;
  }

  auto max_path_width =
      std::ranges::max(assigned_routes | std::views::keys |
                       std::views::transform([](const auto &path) {
                         return path.size();
                       }));

  auto max_controller_width =
      std::ranges::max(route_controllers | std::views::values |
                       std::views::transform([](const auto &controller) {
                         return controller.size();
                       }));

  out << std::format("\n{:-<{}s}\n", "", max_path_width + max_controller_width + 10);
  out << std::format("| {:<{}} | {:<{}} |\n", "Registered Routes", max_path_width, "Controller", max_controller_width);
  out << std::format("{:-<{}s}\n", "", max_path_width + max_controller_width + 10);

  for (const auto& path : assigned_routes | std::views::keys) {
    auto controller_name = route_controllers.at(path);
    out << std::format("| {:<{}} | {:<{}} |\n", path, max_path_width, controller_name, max_controller_width);
  }

  out << std::format("{:-<{}s}\n\n", "", max_path_width + max_controller_width + 10);
}

} // namespace Routing