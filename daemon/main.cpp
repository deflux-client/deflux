#include <coroutine>

#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "tcp_server.hpp"

class daemon : public deflux::net::tcp_server {
public:
    daemon() : tcp_server(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2347)) {}

    void on_message_received(std::vector<uint8_t> raw_message, std::shared_ptr<deflux::net::tcp_connection> connection) override {
        const std::string message{ raw_message.begin(), raw_message.end() };
        spdlog::debug("connection {}: {}", connection->id(), message);

        connection->send(raw_message);
    }

    void on_connection_close(deflux::net::tcp_connection::id_t id) override {
        spdlog::info("connection {} closed\n", id);
    }
};

int main() {
    daemon d{};

    d.run();
    spdlog::shutdown();

    return EXIT_SUCCESS;
}
