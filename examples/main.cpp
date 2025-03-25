#include "controller_base.hpp"
#include "request_response.hpp"
#include "router.hpp"
#include "server.hpp"

class AuthMiddleware : public Routing::IMiddleware {
public:
  auto invoke(const Core::Request &request,
              std::function<Core::Response(const Core::Request &)> next)
      -> Core::Response override {
    if (!is_authenticated(request)) {
      using namespace boost::beast;
      http::response<http::string_body> response{http::status::unauthorized,
                                                 request.request.version()};
      response.set(http::field::content_type, "application/json");
      response.body() = R"({"error": "Unauthorized"})";
      response.prepare_payload();
      return response;
    }

    return next(request);
  }

private:
  auto is_authenticated(const Core::Request &req) const -> bool {
    if (auto it = req.request.find(boost::beast::http::field::authorization);
        it == req.request.end()) {
      return false;
    }

    const auto &auth = req.request[boost::beast::http::field::authorization];
    return !auth.empty() && auth.starts_with("Bearer ");
  }
};

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
    boost::json::object obj;
    obj["message"] = "Welcome!";
    obj["method"] = std::string(req.request.method_string());
    return ok(obj);
  }

  auto about(const Core::Request &req) -> Core::Response {
    boost::json::object obj;
    obj["info"] = "About endpoint";
    obj["target"] = std::string(req.request.target());
    return ok(obj);
  }
};

int main(int argc, char **argv) {
  return Aspion::Server::main(argc, argv, [](Routing::Router &router) {
    router.use<AuthMiddleware>();
    router.add_controller<HomeController>();
  });
}
