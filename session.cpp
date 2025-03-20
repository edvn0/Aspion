#include "session.hpp"

namespace Session {

auto accept_connections(tcp::acceptor &acceptor,
                        Routing::Router &router) -> void {
  acceptor.async_accept([&acc = acceptor, &route = router](beast::error_code ec,
                                                           tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session::HttpSession>(route, std::move(socket))->start();
    }
    accept_connections(acc, route);
  });
}

auto HttpSession::close_connection() -> void {
  beast::error_code ec;
  tcp_socket.shutdown(tcp::socket::shutdown_send, ec);
  tcp_socket.close();
}

auto HttpSession::process_request() -> void {
  http_response = router.route_request(http_request.target(), http_request);
  http_response.version(http_request.version());
  http_response.set(http::field::connection,
                    http_request.keep_alive() ? "keep-alive" : "close");

  http_response.prepare_payload();

  if (should_use_write_some()) {
    write_response_some();
  } else {
    write_response();
  }
}

} // namespace Session