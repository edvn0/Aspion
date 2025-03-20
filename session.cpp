#include "session.hpp"
#include "rabbit_mq_client.hpp"

namespace Session {

auto accept_connections(Messaging::IBusClient& client, tcp::acceptor &acceptor,
                        Routing::Router &router) -> void {
  acceptor.async_accept([&c = client, &acc = acceptor, &route = router](beast::error_code ec,
                                                           tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session::HttpSession>(route, c, std::move(socket))->start();
    }
    accept_connections(c, acc, route);
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