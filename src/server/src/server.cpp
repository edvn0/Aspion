#include "server.hpp"
#include "log.hpp"
#include "otel_init.hpp"
#include "rabbit_mq_client.hpp"
#include "router.hpp"
#include "session.hpp"

#include <boost/asio.hpp>
#include <lyra/lyra.hpp>
#include <thread>
#include <vector>

namespace Aspion::Server {

struct CLIOptions {
  std::uint32_t num_threads = 4;
  std::string rabbitmq_connection_string;
  std::uint32_t port = 8080;
  std::string otlp_endpoint = "http://localhost:4317";
  std::string service_name = "aspion";
  std::string log_file_path = "aspion.log";
  std::string log_level = "info";
};

auto parse_cli(int argc, char **argv, CLIOptions &opts) -> bool {
  auto cli =
      lyra::cli_parser() |
      lyra::opt(opts.num_threads,
                "threads")["--threads"]["-t"]("Worker thread count") |
      lyra::opt(opts.rabbitmq_connection_string,
                "connection")["--connection-string"]["-c"]
          .required() |
      lyra::opt(opts.otlp_endpoint,
                "otel-endpoint")["--otel-endpoint"]("OTLP gRPC endpoint") |
      lyra::opt(opts.service_name,
                "service-name")["--service-name"]("Service name for traces") |
      lyra::opt(opts.log_file_path, "log-file")["--log-file"]("Log file path") |
      lyra::opt(opts.log_level, "log-level")["--log-level"]("Log level") |
      lyra::opt(opts.port, "port")["--port"]["-p"]("Port to listen on");

  if (auto result = cli.parse({argc, argv}); !result) {
    std::cerr << "Error in command line: " << result.message() << std::endl;
    return false;
  }

  opts.num_threads = std::min(static_cast<std::uint32_t>(opts.num_threads),
                              std::thread::hardware_concurrency());

  if (opts.otlp_endpoint.find("http://") == 0) {
    std::cerr << "OTLP endpoint should not contain http:// prefix" << std::endl;
    return false;
  }

  const auto &log_level = opts.log_level;
  const auto &log_file_path = opts.log_file_path;
  Log::configure(log_level, log_file_path);

  return true;
}
auto run(const CLIOptions &, Routing::Router router) -> int;
auto main(int argc, char **argv,
          std::function<void(Routing::Router &)> register_routes) -> int {
  CLIOptions opts;
  if (!parse_cli(argc, argv, opts))
    return 1;

  Routing::Router router;
  register_routes(router);

  return run(opts, std::move(router));
}

auto run(const CLIOptions &options, Routing::Router router) -> int {
  auto &&[num_threads, rabbitmq_connection_string, port, otlp_endpoint,
          service_name, log_file_path, log_level] = options;
  std::uint32_t count_threads =
      std::min(static_cast<std::uint32_t>(num_threads),
               std::thread::hardware_concurrency());
  auto tracer = init_otel_tracer(otlp_endpoint, service_name);

  boost::asio::io_context rabbit_mq_context;
  boost::asio::executor_work_guard guard =
      boost::asio::make_work_guard(rabbit_mq_context);
  std::jthread rabbit_thread([&rabbit_mq_context] { rabbit_mq_context.run(); });

  try {
    auto client = std::make_shared<Messaging::RabbitMQ::Client>(
        rabbit_mq_context, rabbitmq_connection_string, tracer);

    boost::asio::io_context global_io_context;
    boost::asio::ip::tcp::acceptor acceptor(
        global_io_context,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    router.print_routes();
    Session::accept_connections(*client, acceptor, router);

    std::vector<std::jthread> workers;
    for (auto i = 0U; i < count_threads; ++i)
      workers.emplace_back([&global_io_context] { global_io_context.run(); });

    workers.emplace_back(std::move(rabbit_thread));
    for (auto &t : workers)
      t.join();

  } catch (const std::exception &e) {
    Log::error("Server error: {}", e.what());
  }

  Log::info("Exiting");
  return 0;
}

} // namespace Aspion::Server
