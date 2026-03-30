#include "http/server.hpp"

#include <stdexcept>
#include <iostream>
#include <string>

using namespace http;
using namespace net;

ServerBuilder& ServerBuilder::set_public_dir(std::string&& dir) { public_dir_ = dir; return *this; }

ServerBuilder& ServerBuilder::set_port(in_port_t port) { this->port_ = port; return *this; }

ServerBuilder& ServerBuilder::enable_ipv4() { ipv4_ = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv4() { ipv4_ = false; return *this; }

ServerBuilder& ServerBuilder::enable_ipv6() { ipv6_ = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv6() { ipv6_ = false; return *this; }

ServerBuilder& ServerBuilder::set_request_headers_size_limit(std::size_t limit) { headers_limit_ = limit; return *this; }
ServerBuilder& ServerBuilder::set_request_body_size_limit(std::size_t limit) { body_limit_ = limit; return *this; }
ServerBuilder& ServerBuilder::set_request_target_size_limit(std::size_t limit) { request_target_limit_ = limit; return *this; }

ServerBuilder& ServerBuilder::set_request_timeout(const struct timeval& timeout) { this->timeout_ = timeout; return *this; }

Server ServerBuilder::build() {
    return Server(*this);
}

Server::Server(const ServerBuilder& b)
    : ssock(SocketAddr46(b.port_)),
    parser(b.headers_limit_, b.body_limit_, b.request_target_limit_),
    timeout(b.timeout_),
    is_running(false)
{
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
    is_running = true;
    while (is_running) {
        SocketAddr6 client_addr;
        ClientSocket csock = ssock.accept_connection(client_addr, &timeout);
        Connection conn(std::move(csock));

        auto req = parser.parse_request(conn);

        if (req.has_value()) {
            csock.write("server response\n");
        } else if (req.error() == StatusCode::None) {
            continue; // client closed
        } else {
            send_error_response(csock, req.error());
        }
    }
}

void Server::stop() {
    is_running = false;
}
