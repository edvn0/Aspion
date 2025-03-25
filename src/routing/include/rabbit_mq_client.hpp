#pragma once

#include <string>

#include <boost/asio.hpp>
#include <string>

#include <opentelemetry/sdk/trace/tracer_provider.h>

namespace Messaging {

static constexpr auto exchange_name = "logs_exchange";

struct Message {
  const std::string exchange{exchange_name};
  const std::string routing_key;
  const std::string serialised_request;
  const std::string serialised_response;
  const std::string trace_id;
  const std::string span_id;
};

class IBusClient {
public:
  virtual ~IBusClient() = default;
  virtual auto publish(const Message &) -> void = 0;
};

} // namespace Messaging

namespace Messaging::RabbitMQ {

class Client : public Messaging::IBusClient {
public:
  Client(boost::asio::io_context &, const std::string &,
         opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>
             tracer_instance);
  ~Client() override;

  auto publish(const Message &) -> void override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace Messaging::RabbitMQ
