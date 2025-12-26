#include "http/server.hpp"

#include <stdexcept>
#include <iostream>
#include <string>

using namespace http;
using namespace net;

ServerBuilder& ServerBuilder::set_public_dir(std::string&& dir) { public_dir = dir; return *this; }

ServerBuilder& ServerBuilder::set_port(in_port_t port) { this->port = port; return *this; }

ServerBuilder& ServerBuilder::enable_ipv4() { ipv4 = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv4() { ipv4 = false; return *this; }

ServerBuilder& ServerBuilder::enable_ipv6() { ipv6 = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv6() { ipv6 = false; return *this; }

ServerBuilder& ServerBuilder::set_request_headers_size_limit(std::size_t limit) { headers_limit = limit; return *this; }
ServerBuilder& ServerBuilder::set_request_body_size_limit(std::size_t limit) { body_limit = limit; return *this; }

ServerBuilder& ServerBuilder::set_request_timeout(const struct timeval& timeout) { this->timeout = timeout; return *this; }

Server ServerBuilder::build() {
    return Server(*this);
}

Server::Server(const ServerBuilder& b)
    : ssock(SocketType::STREAM, SocketAddr46(b.port)), parser(b.headers_limit, b.body_limit), timeout(b.timeout) {
    /*if (!b.ipv4 && !b.ipv6) {
        throw std::runtime_error("At least one from ipv4 and ipv6 flags has to be enabled.");
    } else if (b.ipv4 && !b.ipv6) {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr4(b.port));
    } else if (!b.ipv4 && b.ipv6) {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr6(b.port));
    } else {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr46(b.port));
    }*/

    // public dir
}

void Server::run() {
    while (true) {
        SocketAddr6 client_addr;
        ClientSocket csock = ssock.accept_connection(client_addr, &timeout);

        Request req = parser.parse_request(&csock);

        if (!req.host) {
            send_error_response(csock, StatusCode::BAD_REQUEST, "Missing Host Header");
        } else {
            csock.write("server response\n", 16);
        }

        /*
        std::cout << "----- CLIENT REQUEST -----" << std::endl;
        std::cout << (int)req.method << " " << req.relative_path << std::endl;
        std::cout << req.keep_alive << std::endl;
        std::cout << req.close << std::endl;
        std::cout << req.host << std::endl;
        std::cout << '"' << req.body << '"' << std::endl;
        */

    }
}
