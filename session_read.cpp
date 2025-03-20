#include "session.hpp"

#include "log.hpp"

namespace Session {

void HttpSession::read_request() {
  flat_buffer.consume(flat_buffer.size());

  if (should_use_read_some()) {
    read_request_some();
  } else {
    read_request_full();
  }
}

auto HttpSession::should_use_read_some() const -> bool {
  auto content_length =
      http_request.request[boost::beast::http::field::content_length];
  auto content_type =
      http_request.request[boost::beast::http::field::content_type];

  return (!content_length.empty() &&
          std::stoul(content_length) > READ_SOME_THRESHOLD) ||
         content_type == "multipart/form-data" ||
         content_type == "application/octet-stream" ||
         content_type == "video/mp4";
}

auto HttpSession::read_request_full() -> void {
  auto self = shared_from_this();
  http::async_read(tcp_socket, flat_buffer, http_request.request,
                   beast::bind_front_handler(&HttpSession::on_read, self));
}

auto HttpSession::read_request_some() -> void {
  auto self = shared_from_this();
  tcp_socket.async_read_some(
      flat_buffer.prepare(READ_SOME_BUFFER_CHUNK_SIZE),
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        self->on_read_some(ec, bytes_transferred);
      });
}

auto HttpSession::on_read(beast::error_code ec, std::size_t) -> void {
  if (ec == beast::http::error::end_of_stream ||
      ec == boost::asio::error::eof) {
    close_connection();
    return;
  } else if (ec) {
    Log::error("Read error: {}", ec.message());
    close_connection();
    return;
  }

  process_request();
}

auto HttpSession::on_read_some(beast::error_code ec,
                               std::size_t bytes_transferred) -> void {
  if (ec == beast::http::error::end_of_stream ||
      ec == boost::asio::error::eof) {
    close_connection();
    return;
  } else if (ec) {
    Log::error("Read error: {}", ec.message());
    close_connection();
    return;
  }

  flat_buffer.commit(bytes_transferred);

  beast::error_code parse_error;
  http::request_parser<http::string_body> parser;
  parser.put(flat_buffer.data(), parse_error);

  if (parse_error == http::error::need_more) {
    read_request_some();
    return;
  } else if (parse_error) {
    Log::error("HTTP parse error: {}", parse_error.message());
    close_connection();
    return;
  }

  http_request = parser.release();
  process_request();
}

} // namespace Session