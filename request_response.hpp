#pragma once

#include <boost/beast/http.hpp>

namespace Core {

using Request = boost::beast::http::request<boost::beast::http::string_body>;
using Response = boost::beast::http::response<boost::beast::http::string_body>;
using RouteHandler = std::function<Response(Request const &)>;

} // namespace Core