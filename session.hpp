#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "request_response.hpp"
#include "util.hpp"
#include "router.hpp"

namespace Session {
    
    using namespace boost::asio;
using tcp = ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

auto accept_connections(tcp::acceptor &acceptor, Routing::Router &router) -> void;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
  tcp::socket tcp_socket;
  beast::flat_buffer flat_buffer;
  Core::Request http_request;
  Core::Response http_response;
  Routing::Router &router;
  static constexpr std::size_t READ_SOME_THRESHOLD = 8192; // 8KB threshold
  static constexpr std::size_t READ_SOME_BUFFER_CHUNK_SIZE =
      4096; // 4KB threshold

public:
  explicit HttpSession(Routing::Router &r, tcp::socket socket)
      : tcp_socket(std::move(socket)), router(r) {}

  void start() { read_request(); }

private:
  auto read_request()-> void;
  auto should_use_read_some() const -> bool;
  auto read_request_full() -> void;
  auto read_request_some() -> void ;
  auto on_read(beast::error_code ec, std::size_t) -> void;
  auto on_read_some(beast::error_code ec,
                    std::size_t bytes_transferred) -> void;
  auto process_request() -> void;
  auto should_use_write_some() -> bool;
  auto write_response() -> void;
  auto write_response_some() -> void;
  auto on_write(beast::error_code ec, std::size_t) -> void;
  auto close_connection() -> void;
};


} // namespace Session