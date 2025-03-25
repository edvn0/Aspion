#pragma once

#include <boost/beast/http.hpp>
#include <boost/json.hpp>

#include "forward.hpp"
#include "request_response.hpp"

namespace Controller {

class IController {
public:
  virtual ~IController() = default;
  virtual void map_routes(Routing::Router &, const std::string &) = 0;
};

namespace http = boost::beast::http;

template <typename Derived> class ControllerBase : public IController {
public:
  ~ControllerBase() override = default;

  void map_routes(Routing::Router &, const std::string &) override = 0;

private:
  template <typename Body>
  static auto create_response(http::status status,
                              const std::string_view content,
                              std::string content_type = "text/plain")
      -> Core::Response {
    http::response<Body> response(status, 11);
    response.set(http::field::content_type, content_type);
    response.body() = content;
    response.prepare_payload();
    return response;
  }

  template <typename JsonType>
  static auto create_response(http::status status, const JsonType &json)
      -> Core::Response {
    return create_response<http::string_body>(
        status, boost::json::serialize(json), "application/json");
  }

  static auto create_response(http::status status,
                              const boost::json::object &json)
      -> Core::Response {
    return create_response<http::string_body>(
        status, boost::json::serialize(json), "application/json");
  }

protected:
  template <typename Body = http::string_body>
  static auto ok(const std::string_view content) -> Core::Response {
    return create_response<Body>(http::status::ok, content);
  }

  static auto ok(const boost::json::value &json) -> Core::Response {
    return create_response<http::string_body>(
        http::status::ok, boost::json::serialize(json), "application/json");
  }

  template <typename Body = http::string_body>
  static auto not_found(const std::string_view content = "Not Found")
      -> Core::Response {
    return create_response<Body>(http::status::not_found, content);
  }

  static auto not_found(const boost::json::value &json) -> Core::Response {
    return create_response<http::string_body>(http::status::not_found,
                                              boost::json::serialize(json),
                                              "application/json");
  }

  template <typename Body = http::string_body>
  static auto bad_request(const std::string_view content = "Bad Request")
      -> Core::Response {
    return create_response<Body>(http::status::bad_request, content);
  }

  static auto bad_request(const boost::json::value &json) -> Core::Response {
    return create_response<http::string_body>(http::status::bad_request,
                                              boost::json::serialize(json),
                                              "application/json");
  }

  static auto unauthorized(const std::string_view content = "Unauthorized")
      -> Core::Response {
    return create_response<http::string_body>(http::status::unauthorized,
                                              content);
  }

  friend class Routing::Router;
};

} // namespace Controller
