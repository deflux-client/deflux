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

#include "io_pool.hpp"

namespace deflux {

void run_executor(asio::io_context& ctx)
{
    const asio::executor_work_guard idle_work = make_work_guard(ctx);
    ctx.run();
}

void io_pool::run() const
{
    assert(!m_executors.empty());
    std::vector<std::thread> execution_threads{};

    for (auto i = 0; i < m_pool_size - 1; i++) {
        asio::io_context& ctx = *m_executors.at(i);
        execution_threads.emplace_back([&, this] { run_executor(ctx); });
    }

    run_executor(*m_executors.back());

    for (auto& thread : execution_threads)
        if (thread.joinable())
            thread.join();
}

void io_pool::stop() const
{
    for (const auto& ctx : m_executors) {
        ctx->stop();
    }
}

asio::io_context& io_pool::get_executor()
{
    auto& ctx = *m_executors.at(m_current_executor);
    m_current_executor = (m_current_executor + 1) % m_pool_size;

    return ctx;
}

size_t io_pool::size() const
{
    return m_pool_size;
}

void io_pool::instantiate_executors()
{
    assert(m_pool_size);

    for (auto i = 0; i < m_pool_size; i++) {
        m_executors.push_back(std::make_unique<asio::io_context>());
    }
}

}
