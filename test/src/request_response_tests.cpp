#include <boost/beast/http.hpp>
#include <doctest/doctest.h>

#include "request_response.hpp"

#include <string>
#include <unordered_map>

namespace http = boost::beast::http;

TEST_CASE("Core::Request path parameter manipulation") {
  http::request<http::string_body> req{http::verb::get, "/", 11};
  Core::Request request(req);
  std::unordered_map<std::string, std::string> params{
      {"id", "123"}, {"name", "alice"}, {"flag", "true"}};
  request.clear_and_set_path_parameters(params);

  auto id_str = request.get_path_param("id");
  CHECK(id_str.has_value());
  CHECK(id_str.value() == "123");

  auto id_int = request.get_path_param<int>("id");
  CHECK(id_int.has_value());
  CHECK(*id_int == 123);

  auto name_sv = request.get_path_param("name");
  CHECK(name_sv.has_value());
  CHECK(name_sv.value() == "alice");

  auto name_str = request.get_path_param<std::string>("name");
  CHECK(name_str.has_value());
  CHECK(*name_str == "alice");

  auto flag = request.get_path_param<bool>("flag");
  CHECK(flag.has_value());
  CHECK(*flag == true);

  auto missing = request.get_path_param("missing");
  CHECK_FALSE(missing.has_value());
}

TEST_CASE("Core::Request header checking") {
  http::request<http::string_body> req{http::verb::get, "/", 11};
  req.set(http::field::host, "example.com");
  Core::Request request(req);
  CHECK(request.has(http::field::host));
  CHECK_FALSE(request.has(http::field::user_agent));
}

TEST_CASE("Core::Response construction") {
  http::response<http::string_body> res{http::status::ok, 11};
  res.body() = "Hello, world!";
  res.prepare_payload();
  Core::Response response(res);
  CHECK(response.response.body() == "Hello, world!");
}
