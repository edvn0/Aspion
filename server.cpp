#include "session.hpp"
#include "controller_base.hpp"
#include "request_response.hpp"
#include "router.hpp"

#include <lyra/lyra.hpp>

class HomeController : public Controller::ControllerBase<HomeController> {
public:
  void map_routes(Routing::Router &router, const std::string& controller_name) override {
    router.add_route("/",
                     [this](const Core::Request &req) { return home(req); }, controller_name);
    router.add_route("/about",
                     [this](const Core::Request &req) { return about(req); }, controller_name);
  }

  auto home(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
      boost::beast::http::status::ok, "Welcome to the Home Page! Method: " +
                              std::string(req.method_string()));
  }

  auto about(const Core::Request &req) -> Core::Response {
    return create_response<boost::beast::http::string_body>(
      boost::beast::http::status::ok, "This is a C++ HTTP server. You requested: " +
                              std::string{req.target()});
  }
};

int main(int argc, char **argv) {
  int num_threads = 4;

  auto cli = lyra::cli_parser() |
             lyra::opt(num_threads,
                       "threads")["--threads"]["-t"]("Worker thread count");

  if (auto result = cli.parse({argc, argv}); !result) {
    std::cerr << "Error: " << result.message() << "\n";
    return 1;
  }

  num_threads = std::min(static_cast<std::uint32_t>(num_threads),
                         std::thread::hardware_concurrency());

  try {
    boost::asio::io_context global_io_context;
    boost::asio::ip::tcp::acceptor acceptor(global_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));

    Routing::Router router;
    router.add_controller<HomeController>();

    router.print_routes(std::cout);

    Session::accept_connections(acceptor, router);
    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i) {
      workers.emplace_back([&io = global_io_context] { io.run(); });
    }

    for (auto &worker : workers) {
      worker.join();
    }
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << "\n";
  }

  return 0;
}
