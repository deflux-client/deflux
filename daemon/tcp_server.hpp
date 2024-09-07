#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <asio/ip/tcp.hpp>

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
    explicit tcp_server(const asio::ip::tcp::endpoint& endpoint) : m_acceptor(m_executor, endpoint) {  }
    virtual ~tcp_server() = default;

    void start_listening();
    void stop_listening();

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
    asio::io_context m_executor;
    asio::ip::tcp::acceptor m_acceptor;

    asio::awaitable<void> handle_new_connections();
};

}

#endif //TCP_SERVER_HPP
