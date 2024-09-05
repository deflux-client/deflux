#include "tcp_connection.hpp"

#include <iostream>
#include <asio/read.hpp>
#include <asio/redirect_error.hpp>

namespace deflux::net {

void tcp_connection::shutdown(const asio::ip::tcp::socket::shutdown_type& direction) {
    if (!m_socket.is_open())
        return;

    m_socket.shutdown(direction);
}

bool tcp_connection::is_open() const {
    return m_socket.is_open();
}

uint32_t tcp_connection::id() const {
    return m_id;
}

void tcp_connection::handle_socket_error(const asio::error_code& ec) {
    std::cout << std::format("Error during socket operation: {}\n", ec.message());
    shutdown(asio::ip::tcp::socket::shutdown_both);
}

asio::awaitable<void> tcp_connection::async_read() {
    while (true) {
        // each message is prepended by it's length, encoded as a 16-bit number
        std::array<uint8_t, 2> message_size_buf{};
        std::vector<uint8_t> message_buf{};
        asio::error_code ec;

        size_t bytes_read = co_await asio::async_read(m_socket, asio::buffer(message_size_buf),
                                                      asio::transfer_exactly(message_size_buf.size()),
                                                      redirect_error(asio::use_awaitable, ec));

        if (ec) {
            handle_socket_error(ec);
            co_return;
        }

        assert(bytes_read == message_size_buf.size());

        const uint16_t incoming_message_size = ntohs(*reinterpret_cast<uint16_t*>(message_size_buf.data()));
        message_buf.resize(incoming_message_size);

        bytes_read = co_await asio::async_read(m_socket, asio::buffer(message_buf, incoming_message_size),
            redirect_error(asio::use_awaitable, ec));

        if (ec) {
            handle_socket_error(ec);
            co_return;
        }

        assert(bytes_read == incoming_message_size);
        m_on_message_received(std::move(message_buf), incoming_message_size, m_id);
    }
}

std::shared_ptr<tcp_connection> tcp_connection::make_connection(asio::ip::tcp::socket socket, uint32_t id, const message_callback_t& callback) {
    return std::make_shared<tcp_connection>(std::move(socket), id, callback, private_t{});
}


}
