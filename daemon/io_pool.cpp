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

    for (auto& thread : execution_threads) {
        if (thread.joinable())
            thread.join();
    }
}

void io_pool::stop() const
{
    for (const auto& ctx : m_executors) {
        ctx->stop();
        delete ctx;
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
        auto* ctx = new asio::io_context();
        m_executors.push_back(ctx);
    }
}

}
