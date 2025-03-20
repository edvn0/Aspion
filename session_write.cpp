#include "session.hpp"

#include "log.hpp"

namespace Session {

auto HttpSession::should_use_write_some() -> bool {
  return http_response.response.body().size() > READ_SOME_THRESHOLD ||
         http_response.response[boost::beast::http::field::content_type] ==
             "application/octet-stream" ||
         http_response.response[boost::beast::http::field::content_type] ==
             "video/mp4";
}

auto HttpSession::write_response() -> void {
  auto self = shared_from_this();
  http::async_write(tcp_socket, http_response.response,
                    beast::bind_front_handler(&HttpSession::on_write, self));
}

auto HttpSession::write_response_some() -> void {
  auto self = shared_from_this();
  tcp_socket.async_write_some(
      boost::asio::buffer(http_response.response.body().data(),
                          http_response.response.body().size()),
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        self->on_write(ec, bytes_transferred);
      });
}

auto HttpSession::on_write(beast::error_code ec, std::size_t) -> void {
  if (ec) {
    Log::error("Write error: {}", ec.message());
    close_connection();
    return;
  }

  if (http_request.request.keep_alive()) {
    read_request();
  } else {
    close_connection();
  }
}

} // namespace Session