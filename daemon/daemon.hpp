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

#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <asio/thread_pool.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "tcp_connection.hpp"
#include "tcp_server.hpp"

namespace deflux {

class daemon : public net::tcp_server {
public:
    daemon()
        : tcp_server(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2347))
    {
    }

protected:
    void on_message_received(
        std::vector<uint8_t> raw_message, std::shared_ptr<net::tcp_connection> connection) override;
    void on_connection_close(net::tcp_connection::id_t id) override;

private:
    asio::thread_pool m_thread_pool = { std::thread::hardware_concurrency() };

    void shutdown();
    nlohmann::json handle_client_request(const std::vector<uint8_t>& raw);
};

}

#endif // DAEMON_HPP
