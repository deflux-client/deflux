/*
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Part of the deflux torrenting client.
 * Copyright (c) 2024 @dodicidodici
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <memory>
#include <utility>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/ip/tcp.hpp>

namespace deflux::net {

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
    struct private_t;

public:
    using id_t = uint32_t;
    using message_callback_t = std::function<void(std::vector<uint8_t>, std::shared_ptr<tcp_connection>)>;
    using close_callback_t = std::function<void(id_t)>;

    tcp_connection() = delete;

    /**
     * Internal constructor. Use the static funcion `make_connection` instead.
     */
    tcp_connection(asio::ip::tcp::socket _s, id_t _id, message_callback_t _m, close_callback_t _c, private_t /*p*/)
        : m_socket(std::move(_s)),
          m_id(_id),
          m_on_message_received(std::move(_m)),
          m_on_connection_close(std::move(_c))
    {
        co_spawn(m_socket.get_executor(), async_read(), asio::detached);
    }

    static std::shared_ptr<tcp_connection> make_connection(asio::ip::tcp::socket socket,
        const message_callback_t& message_callback, const close_callback_t& close_callback);

    void close();
    void send(std::vector<uint8_t>&& message);
    [[nodiscard]] bool is_open() const;
    [[nodiscard]] id_t id() const;
    [[nodiscard]] asio::ip::tcp::endpoint remote_endpoint() const;

private:
    // used to force usage of `make_connection`
    struct private_t { };

    asio::ip::tcp::socket m_socket;
    id_t m_id;
    message_callback_t m_on_message_received;
    close_callback_t m_on_connection_close;

    asio::awaitable<void> async_read();
    asio::awaitable<void> async_write(std::vector<uint8_t> message);
    void handle_socket_error(const asio::error_code& ec);
};

}

#endif // TCP_CONNECTION_HPP
