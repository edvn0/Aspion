#include "rabbit_mq_client.hpp"
#include "server.hpp"
#include "otel_init.hpp"

int main(int argc, char **argv) {
  int num_threads = 4;
  std::string rabbitmq_connection_string;
  std::string otlp_endpoint = "http://localhost:4317";
  std::string service_name = "aspion";

  auto cli = lyra::cli_parser() |
             lyra::opt(num_threads, "threads")["--threads"]["-t"]("Worker thread count") |
             lyra::opt(rabbitmq_connection_string, "connection")["--connection-string"]["-c"]
                 .required() |
             lyra::opt(otlp_endpoint, "otel-endpoint")["--otel-endpoint"]("OTLP gRPC endpoint") |
             lyra::opt(service_name, "service-name")["--service-name"]("Service name for traces");

  if (auto result = cli.parse({argc, argv}); !result) {
    std::cerr << "Error: " << result.message() << "\n";
    return 1;
  }

  num_threads = std::min(static_cast<std::uint32_t>(num_threads),
                         std::thread::hardware_concurrency());

  std::cout << "Connection string: '{" << rabbitmq_connection_string << "}'\n";

  auto tracer = init_otel_tracer(otlp_endpoint, service_name);

  boost::asio::io_context rabbit_mq_context;
  boost::asio::executor_work_guard guard = boost::asio::make_work_guard(rabbit_mq_context);

  std::shared_ptr<Messaging::RabbitMQ::Client> client;
  std::jthread rabbit_thread([&rabbit_mq_context] { rabbit_mq_context.run(); });

  try {
    auto tracer = init_otel_tracer(otlp_endpoint, service_name);
    client = std::make_shared<Messaging::RabbitMQ::Client>(rabbit_mq_context, rabbitmq_connection_string, tracer);

    boost::asio::io_context global_io_context;
    boost::asio::ip::tcp::acceptor acceptor(
        global_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));

    Routing::Router router;
    router.add_controller<HomeController>();

    {
      std::stringstream out;
      router.print_routes(out);
      std::cout << out.str() << "\n";
    }

    Session::accept_connections(*client, acceptor, router);

    std::vector<std::jthread> workers;
    for (int i = 0; i < num_threads; ++i) {
      workers.emplace_back([&global_io_context] { global_io_context.run(); });
    }
    workers.emplace_back(std::move(rabbit_thread));

    for (auto &t : workers) t.join();

  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << "\n";
  }

  std::cout << "[Aspion] Exiting\n";

  client.reset(); // triggers graceful shutdown
  guard.reset();  // allow io_context to exit
  rabbit_thread.join();

  return 0;
}