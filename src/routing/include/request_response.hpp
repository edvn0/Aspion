#pragma once

#include <boost/beast/http.hpp>

#include "util.hpp"

namespace Core {

class Request {
private:
  using Req = boost::beast::http::request<boost::beast::http::string_body>;
  std::unordered_map<std::string, std::string,
                     Util::TransparentHash<std::string_view>, std::equal_to<>>
      path_params;

public:
  Req request;

  template <class... Args>
  Request(Args &&...args) : request(std::forward<Args>(args)...) {}

  Request(const Req &req) : request(req) {}

  template <class H, class C>
  auto clear_and_set_path_parameters(
      const std::unordered_map<std::string, std::string, H, C> &params) {
    path_params.clear();
    for (auto &&[k, v] : params) {
      path_params[k] = v;
    }
  }

  auto get_path_param(const std::string_view name) const
      -> std::optional<std::string_view> {
    if (path_params.contains(name)) {
      return path_params.find(name)->second;
    }
    return std::nullopt;
  }

  auto has(const boost::beast::http::field field) const -> bool {
    return request.find(field) != request.end();
  }

  auto get_path_params() const -> const auto & { return path_params; }
};

class Response {
private:
  using Res = boost::beast::http::response<boost::beast::http::string_body>;

public:
  Res response;

  template <class... Args>
  Response(Args &&...args) : response(std::forward<Args>(args)...) {}

  Response(const Res &req) : response(req) {}
};

using RouteHandler = std::function<Response(Request const &)>;

} // namespace Core
