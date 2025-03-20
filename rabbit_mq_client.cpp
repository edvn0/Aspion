#include "rabbit_mq_client.hpp"

#include "log.hpp"

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>

#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/trace/semantic_conventions.h>
#include <opentelemetry/trace/tracer.h>

#include <thread>

namespace Messaging::RabbitMQ {

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

struct Client::Impl {
  boost::asio::io_context &io_context;
  AMQP::LibBoostAsioHandler io_handler;
  AMQP::TcpConnection io_connection;
  AMQP::TcpChannel io_channel;
  bool ready = false;
  std::mutex mutex;
  std::condition_variable cv;
  nostd::shared_ptr<trace::Tracer> tracer;

  Impl(boost::asio::io_context &context, const std::string &connection_string,
       opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>
           tracer_instance)
      : io_context(context), io_handler(context),
        io_connection(&io_handler, AMQP::Address(connection_string)),
        io_channel(&io_connection), tracer(std::move(tracer_instance)) {

    Log::info("[RabbitMQ] Client::Impl constructor called");

    io_channel.onReady([this, str = connection_string]() {
      Log::info("[RabbitMQ] Channel is ready");

      io_channel.declareQueue("logs_queue", AMQP::durable)
          .onSuccess([](const std::string &name, uint32_t message_count,
                        uint32_t consumer_count) {
            Log::info(std::format(
                "[RabbitMQ] Queue declared: '{}' ({} msgs, {} consumers)", name,
                message_count, consumer_count));
          })
          .onError([](const char *msg) {
            Log::error(
                std::format("[RabbitMQ] Failed to declare queue: {}", msg));
          });

      io_channel.declareExchange("logs_exchange", AMQP::topic, AMQP::durable)
          .onSuccess([]() {
            Log::info("[RabbitMQ] Exchange declared: 'logs_exchange'");
          })
          .onError([](const char *msg) {
            Log::error(
                std::format("[RabbitMQ] Failed to declare exchange: {}", msg));
          });

      io_channel.bindQueue("logs_exchange", "logs_queue", "logs.*")
          .onSuccess([]() {
            Log::info("[RabbitMQ] Queue bound to exchange with routing key");
          })
          .onError([](const char *msg) {
            Log::error(std::format("[RabbitMQ] Failed to bind queue: {}", msg));
          });

      Log::info(std::format("[RabbitMQ] Connected to broker at {}", str));

      {
        std::lock_guard<std::mutex> lock(mutex);
        ready = true;
      }
      cv.notify_all();
    });

    io_channel.onError([](const char *msg) {
      Log::error(std::format("[RabbitMQ] Channel Error: {}", msg));
    });

    Log::info("[RabbitMQ] Connection initialized, waiting for channel to be "
              "ready...");

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(10), [this] { return ready; });

    Log::info("[RabbitMQ] Constructor completed");
  }

  ~Impl() {
    shutdown();
    io_context.stop();
  }

  auto publish_impl(std::string_view exchange, std::string_view routing_key,
                    std::string_view req, std::string_view res,
                    std::string_view trace_id,
                    std::string_view span_id) -> void {

    auto span = tracer->StartSpan(
        "rabbitmq.publish",
        {{"messaging.system", "rabbitmq"},
         {"messaging.destination", std::string(exchange)},
         {"messaging.destination_kind", "topic"},
         {"messaging.rabbitmq.routing_key", std::string(routing_key)},
         {"messaging.operation", "publish"},
         {"trace_id", std::string(trace_id)},
         {"span_id", std::string(span_id)}});

    opentelemetry::trace::Scope scope(span);

    std::string message = std::format(
        R"({{"req":"{}","res":"{}","trace_id":"{}","span_id":"{}"}})", req, res,
        trace_id, span_id);

    if (io_channel.publish(exchange, routing_key, message, AMQP::mandatory)) {
      Log::info(std::format("[RabbitMQ] Published message to {}/{}", exchange,
                            routing_key));
      span->AddEvent("publish.success");
    } else {
      Log::error("[RabbitMQ] Failed to publish message");
      span->AddEvent("publish.failed");
      span->SetStatus(trace::StatusCode::kError, "Message failed to publish");
    }
    span->End();
  }

  auto shutdown() -> void {
    std::lock_guard<std::mutex> lock(mutex);
    if (ready) {
      Log::info("[RabbitMQ] Gracefully shutting down");
      io_channel.close();
      io_connection.close();
      ready = false;
    }
  }
};

Client::Client(
    boost::asio::io_context &context, const std::string &connection_string,
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
    : impl(std::make_unique<Impl>(context, connection_string,
                                  std::move(tracer))) {}

Client::~Client() = default;

auto Client::publish(const Message &message) -> void {
  impl->publish_impl(message.exchange, message.routing_key,
                     message.serialised_request, message.serialised_response,
                     message.trace_id, message.span_id);
}

} // namespace Messaging::RabbitMQ
