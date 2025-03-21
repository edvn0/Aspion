#include "session.hpp"
#include "rabbit_mq_client.hpp"

namespace Session {

auto accept_connections(Messaging::IBusClient &client, tcp::acceptor &acceptor,
                        Routing::Router &router) -> void {
  acceptor.async_accept([&c = client, &acc = acceptor, &route = router](
                            beast::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session::HttpSession>(route, c, std::move(socket))
          ->start();
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
  http_response =
      router.route_request(http_request.request.target(), http_request);
  http_response.response.version(http_request.request.version());
  http_response.response.set(http::field::connection,
                             http_request.request.keep_alive() ? "keep-alive"
                                                               : "close");

  http_response.response.prepare_payload();

  const auto trace_id = Util::generate_uuid();
  const auto span_id = Util::generate_uuid();

  http_request.request.set("X-Trace-ID", trace_id);
  http_response.response.set("X-Trace-ID", trace_id);

  // Publish request/response to RabbitMQ
  Messaging::Message message{
      .exchange = "logs_exchange",
      .routing_key = "logs.info",
      .serialised_request = http_request.request.body(),
      .serialised_response = http_response.response.body(),
      .trace_id = trace_id,
      .span_id = span_id,
  };
  client.publish(message);

  if (should_use_write_some()) {
    write_response_some();
  } else {
    write_response();
  }
}

} // namespace Session