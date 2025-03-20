#include "rabbit_mq_client.hpp"

namespace Messaging::RabbitMQ {

Client::Client(boost::asio::io_context &context,
               const std::string &connection_string)
    : io_context(context), io_handler(context),
      io_connection(&io_handler, AMQP::Address(connection_string)),
      io_channel(&io_connection) {
  std::cout << "[RabbitMQ] Connected to " << connection_string << std::endl;
}

auto Client::publish_impl(const std::string_view exchange,
                          const std::string_view routing_key,
                          const std::string_view req,
                          const std::string_view res,
                          const std::string_view trace_id,
                          const std::string_view span_id) -> void {
  std::string message =
      std::format(R"({{"req":"{}","res":"{}","trace_id":"{}","span_id":"{}"}})",
                  req, res, trace_id, span_id);

  io_channel.publish(exchange, routing_key, message);
  std::cout << std::format("[RabbitMQ] Published message to {}/{}\n", exchange,
                           routing_key);
}

} // namespace Messaging::RabbitMQ