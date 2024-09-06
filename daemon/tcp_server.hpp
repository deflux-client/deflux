#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <asio/ip/tcp.hpp>

#include "tcp_connection.hpp"

namespace deflux::net {

class tcp_server {
public:
    explicit tcp_server(const asio::ip::tcp::endpoint& endpoint) : m_acceptor(m_executor, endpoint) {  }
    virtual ~tcp_server() = default;

    void start_listening();
    void stop_listening();

    virtual void on_message_received(std::vector<uint8_t> raw_message, tcp_connection::id_t id) = 0;
    virtual void on_connection_close(tcp_connection::id_t id) = 0;

protected:
    std::unordered_map<tcp_connection::id_t, std::shared_ptr<tcp_connection>> m_connections{};

private:
    asio::io_context m_executor;
    asio::ip::tcp::acceptor m_acceptor;

    asio::awaitable<void> handle_new_connections();
};

}

#endif //TCP_SERVER_HPP
