#include "tcp_server.hpp"

#include <asio/redirect_error.hpp>
#include <utility>
#include <spdlog/spdlog.h>

namespace deflux::net {

void tcp_server::run() {
    co_spawn(m_acceptor.get_executor(), handle_new_connections(), asio::detached);
    co_spawn(m_set.get_executor(), handle_signals(), asio::detached);
    m_pool.run();
}

void tcp_server::stop() {
    post(m_acceptor.get_executor(), [this] {
        m_acceptor.close();
        m_pool.stop();
    });
}

asio::awaitable<void> tcp_server::handle_new_connections() {
    asio::error_code ec;
    const tcp_connection::close_callback_t on_close = [this](tcp_connection::id_t id) { this->on_connection_close(id); };
    const tcp_connection::message_callback_t on_message = [this](std::vector<uint8_t> m, std::shared_ptr<tcp_connection> connection) {
        this->on_message_received(std::move(m), std::move(connection));
    };

    while (true) {
        asio::ip::tcp::socket sock = co_await m_acceptor.async_accept(m_pool.get_executor(),
            redirect_error(asio::use_awaitable, ec));

        if (ec) {
            spdlog::error("handle_new_connections(): error while trying to accept new connection: {}", ec.message());
            co_return; // TODO: better handling of stuff
        }

        spdlog::info("new incoming connection from {}", sock.remote_endpoint().address().to_string());
        const auto connection = tcp_connection::make_connection(std::move(sock), on_message, on_close);

        m_connections.insert({ connection->id(), connection });
    }
}

asio::awaitable<void> tcp_server::handle_signals() {
    asio::error_code unused{};
    co_await m_set.async_wait(redirect_error(asio::use_awaitable, unused));
    stop();
}

}
