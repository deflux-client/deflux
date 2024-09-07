#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>

#include "io_pool.hpp"
#include "tcp_connection.hpp"

namespace deflux::net {
/**
 * Abstract class implementing an asynchronous TCP server.
 */
class tcp_server {
public:
    /**
     * Creates a `tcp_server` that listens for connections on `endpoint`
     * @param endpoint listening endpoint
     */
    explicit tcp_server(const asio::ip::tcp::endpoint& endpoint) : m_acceptor(m_pool.get_executor(), endpoint) {
        m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_set.add(SIGTERM);
        m_set.add(SIGINT);
#if defined(SIGQUIT)
        m_set.add(SIGQUIT);
#endif
    }
    virtual ~tcp_server() = default;

    void run();
    void stop();

    /**
     * Called whenever a message is received from a remote connection
     *
     * @param raw_message the received message in bytes
     * @param connection the connection that sent the message
     */
    virtual void on_message_received(std::vector<uint8_t> raw_message, std::shared_ptr<tcp_connection> connection) = 0;

    /**
     * Called whenever a remote connection is closed
     *
     * @param id which connection was closed
     */
    virtual void on_connection_close(tcp_connection::id_t id) = 0;

private:
    std::unordered_map<tcp_connection::id_t, std::shared_ptr<tcp_connection>> m_connections{};
    io_pool m_pool{};
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_set{ m_pool.get_executor() };

    asio::awaitable<void> handle_new_connections();
    void handle_signals();
};

}

#endif //TCP_SERVER_HPP
