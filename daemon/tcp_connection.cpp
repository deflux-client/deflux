#include "tcp_connection.hpp"

#include <asio/read.hpp>
#include <asio/redirect_error.hpp>
#include <asio/write.hpp>
#include <spdlog/spdlog.h>

namespace deflux::net {

asio::awaitable<void> tcp_connection::async_write(std::vector<uint8_t> message) {
    asio::error_code ec;
    uint16_t message_size = htons(message.size());
    co_await asio::async_write(m_socket, std::array{ asio::buffer(&message_size, sizeof(message_size)), asio::buffer(message, message.size()) },
        redirect_error(asio::use_awaitable, ec));

    if (ec) {
        handle_socket_error(ec);
        co_return;
    }
}

void tcp_connection::send(std::vector<uint8_t>& message) {
    co_spawn(m_socket.get_executor(), async_write(std::move(message)), asio::detached);
}

asio::ip::tcp::endpoint tcp_connection::remote_endpoint() const {
    return m_socket.remote_endpoint();
}

void tcp_connection::close() {
    if (!m_socket.is_open())
        return;

    asio::error_code ec;
    spdlog::debug("closing connection to {} (connection id {})", remote_endpoint().address().to_string(), m_id);

    m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    if (ec)
        spdlog::warn("close(): could not correctly close socket: {}", ec.message()); // TODO: better handling of stuff

    m_on_connection_close(m_id);
}

bool tcp_connection::is_open() const {
    return m_socket.is_open();
}

uint32_t tcp_connection::id() const {
    return m_id;
}

void tcp_connection::handle_socket_error(const asio::error_code& ec) {
    spdlog::error("error during operation on socket: {}", ec.message());
    close();
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
        assert(message_buf.size() == incoming_message_size);

        m_on_message_received(std::move(message_buf), m_id);
    }
}

std::shared_ptr<tcp_connection> tcp_connection::make_connection(asio::ip::tcp::socket socket,
                                                                const message_callback_t& message_callback,
                                                                const close_callback_t& close_callback)
{
    static id_t id_counter = 0;

    return std::make_shared<tcp_connection>(std::move(socket), id_counter++, message_callback, close_callback, private_t{});
}

}
