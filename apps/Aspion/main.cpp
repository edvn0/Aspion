#include "controller_base.hpp"
#include "json.hpp"
#include "request_response.hpp"
#include "router.hpp"
#include "server.hpp"

class AuthMiddleware : public Routing::IMiddleware {
public:
  auto invoke(const Core::Request &request, const Routing::Middleware &next)
      -> Core::Response override {
    if (!is_authenticated(request)) {
      using namespace boost::beast;
      http::response<http::string_body> response{http::status::unauthorized,
                                                 request.request.version()};
      response.set(http::field::content_type, "application/json");
      response.body() = Routing::json({{"error", "Unauthorized"}});
      response.prepare_payload();
      return response;
    }

    return next(request);
  }

private:
  auto is_authenticated(const Core::Request &req) const -> bool {
    if (!req.has(boost::beast::http::field::authorization)) {
      return false;
    }

    const auto &auth = req.request[boost::beast::http::field::authorization];
    return !auth.empty() && auth.starts_with("Bearer ");
  }
};

class HomeController : public Controller::ControllerBase<HomeController> {
public:
  auto map_routes(Routing::Router &router) -> void override {
    router.add_route("/",
                     [this](const Core::Request &req) { return home(req); });
    router.add_route("/about",
                     [this](const Core::Request &req) { return about(req); });
  }

  auto home(const Core::Request &req) const -> Core::Response {
    boost::json::object obj;
    obj["message"] = "Welcome!";
    obj["method"] = std::string(req.request.method_string());
    return ok(obj);
  }

  auto about(const Core::Request &req) const -> Core::Response {
    boost::json::object obj;
    obj["info"] = "About endpoint";
    obj["target"] = std::string(req.request.target());
    return ok(obj);
  }

  // Parse /users/{id} route
  auto user(const Core::Request &req) const -> Core::Response {
    auto id = req.get_path_param<std::uint32_t>("id");
    if (!id) {
      return not_found("User not found");
    }

    static std::unordered_map<std::uint32_t, std::string> users = {
        {1, "Alice"}, {2, "Bob"}, {3, "Charlie"}};

    if (users.contains(*id)) {
      return not_found("User not found");
    }

    using namespace std::string_view_literals;

    Core::Response response;
    response.response.result(boost::beast::http::status::ok);
    response.response.set(boost::beast::http::field::content_type,
                          "application/json");
    response.response.body() = Routing::json(Routing::JSONInput{});
    response.response.prepare_payload();
    return response;
  }
};

int main(int argc, char **argv) {
  return Aspion::Server::start(argc, argv, [](Routing::Router &router) {
    router.use<AuthMiddleware>();
    router.add_controller<HomeController>();
  });
}
