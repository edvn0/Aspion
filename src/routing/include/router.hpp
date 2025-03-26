#pragma once

#include <boost/beast/http.hpp>
#include <boost/type_index.hpp>
#include <functional>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <type_traits>

#include "controller_base.hpp"
#include "middleware.hpp"
#include "request_response.hpp"
#include "util.hpp"

namespace Routing {

namespace http = boost::beast::http;

class Router {
public:
  auto add_route(const std::string &path, Core::RouteHandler handler) {
    if (assigned_routes.contains(path)) {
      throw std::runtime_error(std::format("Path {} already exists.", path));
    }

    auto wrapped_handler = build_pipeline(std::move(handler));
    assigned_routes[path] = std::move(wrapped_handler);
  }

  template <typename T>
    requires std::is_base_of_v<Controller::ControllerBase<T>, T>
  void add_controller() {
    auto controller_name = boost::typeindex::type_id<T>();
    if (controllers.find(controller_name) == std::end(controllers)) {
      auto controller = std::make_unique<T>();
      controller->map_routes(*this);
      controllers[controller_name] = std::move(controller);
    }
  }

  auto route_request(const std::string &path, const Core::Request &req) const {
    if (auto it = assigned_routes.find(path); it != std::end(assigned_routes)) {
      return it->second(req);
    }
    return Controller::ControllerBase<Router>::create_response<
        http::string_body>(http::status::not_found, "404 Not Found");
  }

  template <typename Middleware>
    requires std::is_base_of_v<IMiddleware, Middleware>
  auto use() -> void {
    middlewares.emplace_back(std::make_shared<Middleware>());
  }

  auto print_routes() const -> void;

private:
  using StringHash = Util::TransparentHash<std::string_view>;
  using TypeIndexHash = Util::TransparentHash<boost::typeindex::type_index>;
  std::unordered_map<std::string, Core::RouteHandler, StringHash,
                     std::equal_to<>>
      assigned_routes{};
  std::unordered_map<boost::typeindex::type_index,
                     std::unique_ptr<Controller::IController>, TypeIndexHash,
                     std::equal_to<>>
      controllers{};
  std::vector<std::shared_ptr<Routing::IMiddleware>> middlewares{};

  auto build_pipeline(Core::RouteHandler handler) const -> Core::RouteHandler {
    auto next = std::move(handler);
    for (auto it = std::rbegin(middlewares); it != std::rend(middlewares);
         ++it) {
      auto &middleware = *it;
      Core::RouteHandler current = std::move(next);
      next = [current, middleware](const Core::Request &req) {
        return middleware->invoke(req, current);
      };
    }
    return next;
  }
};

using RegistrationFunction = decltype(+[](Router &) -> void {});
void register_controller(Router &, RegistrationFunction);

} // namespace Routing
