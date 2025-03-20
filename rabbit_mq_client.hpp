#pragma once

#include <string>

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

namespace Messaging {

struct Message {
  const std::string exchange;
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
  Client(boost::asio::io_context &, const std::string &);
  ~Client() override = default;

  auto publish(const Message &message) -> void override {
    return publish_impl(message.exchange, message.routing_key,
                        message.serialised_request, message.serialised_response,
                        message.trace_id, message.span_id);
  }

private:
  boost::asio::io_context &io_context;
  AMQP::LibBoostAsioHandler io_handler;
  AMQP::TcpConnection io_connection;
  AMQP::TcpChannel io_channel;

  auto publish_impl(std::string_view exchange, std::string_view routing_key,
                    std::string_view req, std::string_view res,
                    std::string_view trace_id,
                    std::string_view span_id) -> void;
};

} // namespace Messaging::RabbitMQ