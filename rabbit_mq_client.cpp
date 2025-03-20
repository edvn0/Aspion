#include "rabbit_mq_client.hpp"

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>

namespace Messaging::RabbitMQ {

constexpr int MAX_RETRIES = 5;
constexpr int INITIAL_DELAY_MS = 500; // Start with 500ms, then increase

struct Client::Impl {
  boost::asio::io_context &io_context;
  AMQP::LibBoostAsioHandler io_handler;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard;
  std::unique_ptr<AMQP::TcpConnection> io_connection;
  std::unique_ptr<AMQP::TcpChannel> io_channel;

  Impl(boost::asio::io_context &context, const std::string &connection_string)
      : io_context(context), io_handler(context),
        work_guard(boost::asio::make_work_guard(context)) {

    for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
      try {
        io_connection = std::make_unique<AMQP::TcpConnection>(
            &io_handler, AMQP::Address(connection_string));
        io_channel = std::make_unique<AMQP::TcpChannel>(io_connection.get());

        io_channel->onReady([this, str = connection_string]() {
          io_channel->declareQueue("logs_queue", AMQP::durable);
          io_channel->declareExchange("logs_exchange", AMQP::fanout,
                                      AMQP::durable);
          io_channel->bindQueue("logs_exchange", "logs_queue",
                                "logs.routing.key");
          std::cout << std::format("Connected to RabbitMQ at {}\n", str);
        });

        io_channel->onError([](char const *msg) {
          std::cout << std::format("Channel error: {}\n", msg);
        });

        std::cout << std::format(
            "RabbitMQ connection established on attempt {}/{}\n", attempt,
            MAX_RETRIES);
        return; // Exit loop if successful
      } catch (const std::exception &e) {
        std::cerr << std::format(
            "RabbitMQ connection failed (attempt {}/{}): {}\n", attempt,
            MAX_RETRIES, e.what());

        if (attempt < MAX_RETRIES) {
          int delay =
              INITIAL_DELAY_MS * (1 << (attempt - 1)); // Exponential backoff
          std::cerr << std::format("Retrying in {} ms...\n", delay);
          std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        } else {
          throw std::runtime_error(
              "Failed to connect to RabbitMQ after multiple attempts.");
        }
      }
    }
  }

  ~Impl() {
    if (io_channel) {
      io_channel->close();
    }
    if (io_connection) {
      io_connection->close();
    }

    boost::asio::steady_timer timer(io_context);
    timer.expires_after(std::chrono::milliseconds(200));
    timer.async_wait([](auto) {});
    io_context.run_for(std::chrono::milliseconds(250));

    work_guard.reset();
    io_context.stop();
  }

  auto publish_impl(std::string_view exchange, std::string_view routing_key,
                    std::string_view req, std::string_view res,
                    std::string_view trace_id,
                    std::string_view span_id) -> void {
    std::string message = std::format(
        R"({{"req":"{}","res":"{}","trace_id":"{}","span_id":"{}"}})", req, res,
        trace_id, span_id);

    io_channel->publish(exchange, routing_key, message);
    std::cout << std::format("[RabbitMQ] Published message to {}/{}\n",
                             exchange, routing_key);
  }
};

Client::Client(boost::asio::io_context &context,
               const std::string &connection_string)
    : impl(std::make_unique<Impl>(context, connection_string)) {}

Client::~Client() = default;

auto Client::publish(const Message &message) -> void {
  impl->publish_impl(message.exchange, message.routing_key,
                     message.serialised_request, message.serialised_response,
                     message.trace_id, message.span_id);
}

} // namespace Messaging::RabbitMQ