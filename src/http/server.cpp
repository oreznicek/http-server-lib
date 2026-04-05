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
    : ssock_(SocketAddr46(b.port_)),
    parser_(b.headers_limit_, b.body_limit_, b.request_target_limit_),
    timeout_(b.timeout_),
    is_running_(false)
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

void Server::send_error_response(Connection& conn, StatusCode code)
{
    std::string full_message = std::to_string((int)code) + " " + to_string(code);
    std::string resp_body = "<html><body><h1>" + full_message + "</h1></body></html>";
    std::string response =
        "HTTP/1.1 " + full_message + "\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(resp_body.size()) + "\r\n"
        "\r\n" + resp_body;
    conn.send(response);
}

void Server::run()
{
    is_running_ = true;
    while (is_running_) {
        SocketAddr6 client_addr;
        ClientSocket csock = ssock_.poll(client_addr, &timeout_);
        if (!csock.is_valid()) {
            continue;
        }
        Connection conn(std::move(csock));

        auto req = parser_.parse_request(conn);

        if (req.has_value()) {
            csock.send("server response\n");
        } else if (req.error() == StatusCode::None) {
            continue; // client closed
        } else {
            send_error_response(conn, req.error());
        }
    }
}

void Server::stop()
{
    is_running_ = false;
}

SocketAddr4 Server::get_addr()
{
    return SocketAddr4(ssock_.srv_port_);
}
