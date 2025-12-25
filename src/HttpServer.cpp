#include "HttpServer.hpp"

#include <iostream>
#include <format>

HttpServerBuilder& HttpServerBuilder::set_public_dir(std::string&& dir) { public_dir = dir; return *this; }

HttpServerBuilder& HttpServerBuilder::set_port(in_port_t port) { this->port = port; return *this; }

HttpServerBuilder& HttpServerBuilder::enable_ipv4() { ipv4 = true; return *this; }
HttpServerBuilder& HttpServerBuilder::disable_ipv4() { ipv4 = false; return *this; }

HttpServerBuilder& HttpServerBuilder::enable_ipv6() { ipv6 = true; return *this; }
HttpServerBuilder& HttpServerBuilder::disable_ipv6() { ipv6 = false; return *this; }

HttpServerBuilder& HttpServerBuilder::set_request_headers_size_limit(std::size_t limit) { headers_limit = limit; return *this; }
HttpServerBuilder& HttpServerBuilder::set_request_body_size_limit(std::size_t limit) { body_limit = limit; return *this; }
    
HttpServerBuilder& set_request_timeout(const struct timeval& timeout) { this->timeout = timeout; return *this; }

HttpServer HttpServerBuilder::build() {
    return HttpServer(*this);
}

HttpServer::HttpServer(const HttpServerBuilder& b) : parser(b.request_headers_size_limit, b.request_body_size_limit), timeout(b.timeout) {
    if (!b.ipv4 && !b.ipv6) {
        throw std::runtime_error("At least one from ipv4 and ipv6 flags has to be enabled.");
    } else if (b.ipv4 && !b.ipv6) {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr4("127.0.0.1", b.port));
    } else if (!b.ipv4 && b.ipv6) {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr6("::", b.port));
    } else {
        ssock = ServerSocket(SocketType::STREAM, SocketAddr46("::", b.port));
    }

    // public dir
}

void HttpServer::run() {
    while (true) {
        SocketAddr6 client_addr;
        ClientSocket csock = ssock.accept_connection(client_addr, timeout);

        std::string request_message = csock.read();
        std::cout << "----- CLIENT REQUEST -----" << std::endl;
        std::cout << '"' << request_message << '"' << std::endl;

        csock.write("server response\n", 16);
    }
}

void send_error_response(ClientSocket& csock, HttpStatusCode code) {
    std::to_string response =
        std::format("HTTP/1.1 {} {}\r\n", code, to_string()
        "\r\n"
}
