#include "rabbit_mq_client.hpp"

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>

#include <thread>

namespace Messaging::RabbitMQ {

struct Client::Impl {
  boost::asio::io_context &io_context;
  AMQP::LibBoostAsioHandler io_handler;
  AMQP::TcpConnection io_connection;
  AMQP::TcpChannel io_channel;
  bool ready = false;
  std::mutex mutex;
  std::condition_variable cv;

  Impl(boost::asio::io_context &context, const std::string &connection_string)
      : io_context(context), io_handler(context),
        io_connection(&io_handler, AMQP::Address(connection_string)),
        io_channel(&io_connection) {

    std::cout << "[RabbitMQ] Client::Impl constructor called\n";

    io_channel.onReady([this, str = connection_string]() {
      std::cout << "[RabbitMQ] Channel is ready\n";

      io_channel.declareQueue("logs_queue", AMQP::durable);
      io_channel.declareExchange("logs_exchange", AMQP::fanout, AMQP::durable);
      io_channel.bindQueue("logs_exchange", "logs_queue", "logs.routing.key");

      std::cout << std::format("[RabbitMQ] Connected to broker at {}\n", str);

      {
        std::lock_guard<std::mutex> lock(mutex);
        ready = true;
      }
      cv.notify_all();
    });

    io_channel.onError([](const char *msg) {
      std::cerr << std::format("[RabbitMQ] Channel Error: {}\n", msg);
    });

    std::cout << "[RabbitMQ] Connection initialized, waiting for channel to be "
                 "ready...\n";

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(5), [this] { return ready; });

    std::cout << "[RabbitMQ] Constructor completed\n";
  }

  ~Impl() {
    io_channel.close();
    io_connection.close();
    io_context.stop();
  }

  auto publish_impl(std::string_view exchange, std::string_view routing_key,
                    std::string_view req, std::string_view res,
                    std::string_view trace_id, std::string_view span_id)
      -> void {
    // Optionally wait for ready state before publishing
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!ready) {
        cv.wait_for(lock, std::chrono::seconds(5), [this] { return ready; });
        if (!ready) {
          std::cerr << "[RabbitMQ] Timed out waiting for channel to be ready\n";
          return;
        }
      }
    }

    std::string message = std::format(
        R"({{"req":"{}","res":"{}","trace_id":"{}","span_id":"{}"}})", req, res,
        trace_id, span_id);
    io_channel.publish(exchange, routing_key, message);
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