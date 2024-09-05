#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <memory>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/ip/tcp.hpp>
#include <utility>

namespace deflux::net {

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
    struct private_t;

public:
    using message_callback_t = std::function<void(std::vector<uint8_t>, size_t, uint32_t)>;

    tcp_connection() = delete;
    tcp_connection(asio::ip::tcp::socket _s, uint32_t _id, message_callback_t _c, private_t p)
        : m_socket(std::move(_s)), m_id(_id), m_on_message_received(std::move(_c))
    {
       co_spawn(m_socket.get_executor(), async_read(), asio::detached);
    }

    static std::shared_ptr<tcp_connection> make_connection(asio::ip::tcp::socket socket, uint32_t id, const message_callback_t& callback);
    void shutdown(const asio::ip::tcp::socket::shutdown_type& direction);
    [[nodiscard]] bool is_open() const;
    [[nodiscard]] uint32_t id() const;

private:
    struct private_t{};

    asio::ip::tcp::socket m_socket;
    uint32_t m_id;
    message_callback_t m_on_message_received;

    asio::awaitable<void> async_read();
    void handle_socket_error(const asio::error_code& ec);
};

}

#endif //TCP_CONNECTION_HPP
