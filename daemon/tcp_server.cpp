#include "tcp_server.hpp"

#include <asio/redirect_error.hpp>

namespace deflux::net {

void tcp_server::start_listening() {
    co_spawn(m_acceptor.get_executor(), handle_new_connections(), asio::detached);
    m_executor.run();
}

void tcp_server::stop_listening() {
    asio::post(m_executor, [this] {
        m_acceptor.close();
    });
}

asio::awaitable<void> tcp_server::handle_new_connections() {
    asio::error_code ec;
    tcp_connection::close_callback_t on_close = [this](tcp_connection::id_t id) { this->on_connection_close(id); };
    tcp_connection::message_callback_t on_message = [this](std::vector<uint8_t> m, size_t s, tcp_connection::id_t id) {
        this->on_message_received(std::move(m), s, id);
    };

    while (true) {
        asio::ip::tcp::socket sock = co_await m_acceptor.async_accept(m_acceptor.get_executor(), redirect_error(asio::use_awaitable, ec));

        if (ec)
            throw std::runtime_error(std::format("m_acceptor: {}", ec.message()));

        m_connections.push_back(tcp_connection::make_connection(std::move(sock), on_message, on_close));
    }
}

}
