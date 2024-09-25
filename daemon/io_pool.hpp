#ifndef IO_POOL_HPP
#define IO_POOL_HPP

#include <asio/io_context.hpp>
#include <stdexcept>
#include <thread>
#include <vector>

namespace deflux {

/**
 * Pool of multiple asio strands.
 *
 * By default, the number of strands is equal to the amount of hardware threads.
 */
class io_pool {
public:
    io_pool(const io_pool& other) = delete;

    /**
     * Creates an `io_pool` with a default size of std::thread::hardware_concurrency, or 1,
     * if hardware concurrency cannot be determined.
     */
    io_pool()
    {
        instantiate_executors();
    }

    /**
     * Creates an `io_pool` with `size` executors
     *
     * @param size non-zero size of the pool
     * @throws std::invalid_argument if size is 0
     */
    explicit io_pool(const size_t& size)
        : m_pool_size(size)
    {
        if (size == 0)
            throw std::invalid_argument("io_pool size cannot be 0");

        instantiate_executors();
    }

    /**
     * Starts the `io_pool` event loop.
     *
     * @note Spawns `size()` - 1 threads and uses the caller thread as executors.
     */
    void run() const;

    /**
     * Stops the event loop.
     */
    void stop() const;

    /**
     * Gets the next executor to be used. Selected using round robin.
     *
     * @return next executor
     */
    [[nodiscard]] asio::io_context& get_executor();
    [[nodiscard]] size_t size() const;

private:
    // FIXME consider using less threads as a default, say, 75% of available threads
    size_t m_pool_size = std::max<size_t>(1, std::thread::hardware_concurrency());
    size_t m_current_executor = 0;
    // store as a pointer because the copy constructor of asio::io_context is deleted
    std::vector<std::unique_ptr<asio::io_context>> m_executors{};

    void instantiate_executors();
};

}

#endif // IO_POOL_HPP
