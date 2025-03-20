#include "rabbit_mq_client.hpp"
#include "server.hpp"

int main(int argc, char **argv) {
  int num_threads = 4;
  std::string rabbitmq_connection_string;

  auto cli = lyra::cli_parser() |
             lyra::opt(num_threads,
                       "threads")["--threads"]["-t"]("Worker thread count") |
             lyra::opt(rabbitmq_connection_string,
                       "bus connection string")["--connection-string"]["-c"]
                 .required();

  if (auto result = cli.parse({argc, argv}); !result) {
    std::cerr << "Error: " << result.message() << "\n";
    return 1;
  }

  num_threads = std::min(static_cast<std::uint32_t>(num_threads),
                         std::thread::hardware_concurrency());

  std::cout << "Connection string: '{" << rabbitmq_connection_string << "}'\n";

  try {
    boost::asio::io_context global_io_context;
    Messaging::RabbitMQ::Client client{
        global_io_context,
        rabbitmq_connection_string,
    };
    boost::asio::ip::tcp::acceptor acceptor(
        global_io_context,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));

    Routing::Router router;
    router.add_controller<HomeController>();

    std::stringstream out;
    router.print_routes(out);
    std::cout << out.str() << "\n";

    Session::accept_connections(client, acceptor, router);
    std::vector<std::jthread> workers;
    for (int i = 0; i < num_threads; ++i) {
      workers.emplace_back([&io = global_io_context] { io.run(); });
    }
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << "\n";
  }

  return 0;
}