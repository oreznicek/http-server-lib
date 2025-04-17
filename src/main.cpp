#include <iostream>
#include <thread>
#include <poll.h>

#include "Socket.hpp"

#define PORT 8080

int main() {
    // TODO: allow binding to port `80` `setcap` on linux and windows???
    SocketAddr server_addr("127.0.0.1", PORT);
    ServerSocket server_sock(ProtocolFamily::IPV4, SocketType::STREAM, server_addr);

    SocketAddr6 server_addr6("::", PORT);
    ServerSocket server_sock(ProtocolFamily::IPV6, SocketType::STREAM, server_addr6);

    /*struct pollfd server_sockets[2] = {
        { .fd =  }
    };*/

    //int num_threads = std::thread::hardware_concurrency();
    //std::cout << num_threads << std::endl;

    while (true) {
        SocketAddr client_addr;
        ClientSocket client_sock = server_sock.accept_connection(client_addr);

        std::string request_message = client_sock.read();
        std::cout << "----- CLIENT REQUEST -----" << std::endl;
        std::cout << '"' << request_message << '"' << std::endl;

        client_sock.write("server response", 16);
    }
}
