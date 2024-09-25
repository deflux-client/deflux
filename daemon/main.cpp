#include <coroutine>

#include <asio.hpp>
#include <deflux/protocol.hpp>
#include <spdlog/spdlog.h>

#include "tcp_server.hpp"

namespace deflux {

class daemon : public net::tcp_server {
public:
    daemon()
        : tcp_server(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2347))
    {
    }

protected:
    void on_message_received(std::vector<uint8_t> raw_message, std::shared_ptr<net::tcp_connection> connection) override
    {
        auto f = [this, r = move(raw_message), connection = move(connection)]() mutable {
            auto res = nlohmann::json::to_msgpack(handle_client_request(r));
            connection->send(std::move(res));
        };

        // defer execution to another thread and free up the caller strand
        post(m_thread_pool, f);
    }

    void on_connection_close(net::tcp_connection::id_t id) override
    {
        spdlog::info("connection {} closed", id);
    }

private:
    asio::thread_pool m_thread_pool = { std::thread::hardware_concurrency() };

    void shutdown()
    {
        stop();
    }

    nlohmann::json handle_client_request(const std::vector<uint8_t>& raw)
    {
        unparsed_message_t request{};

        try {
            request = nlohmann::json::from_msgpack(raw);
        } catch (const std::exception& e) {
            spdlog::error("error while parsing request: {}", e.what());
            return make_response(error_reason::invalid_request);
        }

        switch (request.type) {
        case message_type::shutdown:
            shutdown();
            [[fallthrough]];
        case message_type::ping:
            return make_response(empty_t{}, request.tag);
        default:
            return make_response(error_reason::no_such_method, request.tag);
        }

        assert(false);
    }
};

}

int main()
{
    deflux::daemon daemon{};

    daemon.run();
    spdlog::shutdown();

    return EXIT_SUCCESS;
}
