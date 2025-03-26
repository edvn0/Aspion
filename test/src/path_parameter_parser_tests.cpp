#include <doctest/doctest.h>

#include "path_parameter_parser.hpp"

using Routing::Util::PathParameterParser;

TEST_CASE("Parse integral types from REST-like paths") {
  SUBCASE("Parse int from /users/0042") {
    std::string path = "/users/0042";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<int> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == 42);
  }

  SUBCASE("Parse int with invalid digits") {
    PathParameterParser<int> parser;
    auto result = parser.parse("abcd");
    CHECK_FALSE(result.has_value());
  }

  SUBCASE("Parse uint8_t from /codes/7") {
    std::string path = "/codes/7";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<std::uint8_t> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == 7);
  }

  SUBCASE("Parse int16_t from /range/42") {
    std::string path = "/range/42";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<int16_t> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == 42);
  }
}

TEST_CASE("Parse floating point types from REST-like paths") {
  SUBCASE("Parse float from /values/3.14") {
    std::string path = "/values/3.14";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<float> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == doctest::Approx(3.14));
  }

  SUBCASE("Parse double from /values/3.141592") {
    std::string path = "/values/3.141592";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<double> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == doctest::Approx(3.141592));
  }
}

TEST_CASE("Parse bool from REST-like paths") {
  SUBCASE("Parse bool true from /flags/true") {
    std::string path = "/flags/true";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<bool> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == true);
  }

  SUBCASE("Parse bool false from /flags/false") {
    std::string path = "/flags/false";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<bool> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == false);
  }

  SUBCASE("Parse bool true from /flags/1") {
    std::string path = "/flags/1";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<bool> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == true);
  }

  SUBCASE("Parse bool false from /flags/0") {
    std::string path = "/flags/0";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<bool> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == false);
  }

  SUBCASE("Parse bool with invalid value from /flags/yes") {
    std::string path = "/flags/yes";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<bool> parser;
    auto result = parser.parse(param);
    CHECK_FALSE(result.has_value());
  }
}

TEST_CASE("Parse string and string_view from REST-like paths") {
  SUBCASE("Parse std::string from /users/johndoe") {
    std::string path = "/users/johndoe";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<std::string> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == "johndoe");
  }

  SUBCASE("Parse std::string_view from /users/janedoe") {
    std::string path = "/users/janedoe";
    auto pos = path.find_last_of('/');
    std::string_view param = path.substr(pos + 1);
    PathParameterParser<std::string_view> parser;
    auto result = parser.parse(param);
    CHECK(result.has_value());
    CHECK(*result == "janedoe");
  }
}
