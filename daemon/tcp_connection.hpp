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
    using id_t = uint32_t;
    using message_callback_t = std::function<void(std::vector<uint8_t>, size_t, id_t)>;
    using close_callback_t = std::function<void(id_t)>;

    tcp_connection() = delete;
    tcp_connection(asio::ip::tcp::socket _s, uint32_t _id, message_callback_t _m, close_callback_t _c, private_t p)
        : m_socket(std::move(_s)), m_id(_id), m_on_message_received(std::move(_m)), m_on_connection_close(std::move(_c))
    {
       co_spawn(m_socket.get_executor(), async_read(), asio::detached);
    }

    static std::shared_ptr<tcp_connection> make_connection(asio::ip::tcp::socket socket,
                                                           const message_callback_t& message_callback,
                                                           const close_callback_t& close_callback);

    void close();
    [[nodiscard]] bool is_open() const;
    [[nodiscard]] id_t id() const;

private:
    struct private_t{};

    asio::ip::tcp::socket m_socket;
    id_t m_id;
    message_callback_t m_on_message_received;
    close_callback_t m_on_connection_close;

    asio::awaitable<void> async_read();
    void handle_socket_error(const asio::error_code& ec);
};

}

#endif //TCP_CONNECTION_HPP
