#include <coroutine>
#include <asio.hpp>

#include "tcp_server.hpp"

class daemon : public deflux::net::tcp_server {
public:
    daemon() : tcp_server(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2347)) {}

    void on_message_received(std::vector<uint8_t> raw_message, size_t size, uint32_t id) override {
        const char* data = (char*)raw_message.data();

        for (int i = 0; i < size; i++) {
            printf("%c", data[i]);
        }

        printf("\n");

        fflush(stdout);
    }

    void on_connection_close(deflux::net::tcp_connection::id_t id) override {
        printf("connection %d closed\n", id);
        fflush(stdout);
    }
};

int main() {
    daemon d{};
    d.start_listening();

    return EXIT_SUCCESS;
}
