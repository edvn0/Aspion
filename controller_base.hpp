#pragma once

#include <boost/beast/http.hpp>
#include "forward.hpp"

namespace Controller {

class IController {
  public:
      virtual ~IController() = default;
      virtual void map_routes(Routing::Router&, const std::string&) = 0;
  };

namespace http = boost::beast::http;

template <typename Derived> class ControllerBase: public IController {
public:
  virtual ~ControllerBase() = default;

  virtual void map_routes(Routing::Router& router, const std::string&) override = 0;

protected:
  template <typename Body>
  static auto
  create_response(http::status status, std::string content,
                  std::string content_type = "text/plain") {
    http::response<Body> response(status, 11);
    response.set(http::field::content_type, content_type);
    response.body() = std::move(content);
    response.prepare_payload();
    return response;
  }

  friend class Routing::Router;
};

} // namespace Controller