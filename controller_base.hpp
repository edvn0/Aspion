#pragma once

#include <boost/beast/http.hpp>

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

protected:
  template <typename Body>
  static auto
  create_response(http::status status, const std::string_view content,
                  std::string content_type = "text/plain") -> Core::Response {
    http::response<Body> response(status, 11);
    response.set(http::field::content_type, content_type);
    response.body() = content;
    response.prepare_payload();
    return response;
  }

  friend class Routing::Router;
};

} // namespace Controller