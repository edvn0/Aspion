#include "controller_base.hpp"
#include "request_response.hpp"
#include "router.hpp"
#include "server.hpp"

class HomeController : public Controller::ControllerBase<HomeController> {
public:
  auto map_routes(Routing::Router &router, const std::string &controller_name)
      -> void override {
    router.add_route(
        "/", [this](const Core::Request &req) { return home(req); },
        controller_name);
    router.add_route(
        "/about", [this](const Core::Request &req) { return about(req); },
        controller_name);
  }

  auto home(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
        boost::beast::http::status::ok,
        "Welcome! Method: " + std::string(req.request.method_string()));
  }

  auto about(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
        boost::beast::http::status::ok,
        "About: " + std::string{req.request.target()});
  }
};

int main(int argc, char **argv) {
  return Aspion::Server::main(argc, argv, [](Routing::Router &router) {
    router.add_controller<HomeController>();
  });
}
