#include "controller_base.hpp"
#include "request_response.hpp"
#include "router.hpp"
#include "session.hpp"

#include <lyra/lyra.hpp>

class HomeController : public Controller::ControllerBase<HomeController> {
public:
  auto map_routes(Routing::Router &router,
                  const std::string &controller_name) -> void override {
    router.add_route(
        "/", [this](const Core::Request &req) { return home(req); },
        controller_name);
    router.add_route(
        "/about", [this](const Core::Request &req) { return about(req); },
        controller_name);
  }

  auto home(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
        boost::beast::http::status::ok, "Welcome to the Home Page! Method: " +
                                            std::string(req.method_string()));
  }

  auto about(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
        boost::beast::http::status::ok,
        "This is a C++ HTTP server. You requested: " +
            std::string{req.target()});
  }
};
