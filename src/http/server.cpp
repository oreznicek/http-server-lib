#include "http/server.hpp"
#include "logger.hpp"

#include <exception>
#include <stdexcept>
#include <string>
#include <thread>

using namespace http;
using namespace net;

ServerBuilder& ServerBuilder::set_public_dir(const std::filesystem::path& public_dir) { router_.public_dir_ = public_dir; return *this; }
ServerBuilder& ServerBuilder::set_error_page_template(std::string&& err_template) { Response::error_template_ = std::move(err_template); return *this; }
ServerBuilder& ServerBuilder::enable_directory_listing() { router_.list_dir_ = true; return *this; }
ServerBuilder& ServerBuilder::disable_directory_listing() { router_.list_dir_ = false; return *this; }

ServerBuilder& ServerBuilder::add_route(std::string&& path, http::RequestMethod method, handlers::Router::HandlerFunc&& func)
{
    router_.add_route(std::move(path), method, std::move(func));
    return *this;
}

ServerBuilder& ServerBuilder::set_port(in_port_t port) { this->port_ = port; return *this; }

ServerBuilder& ServerBuilder::enable_ipv4() { ipv4_ = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv4() { ipv4_ = false; return *this; }

ServerBuilder& ServerBuilder::enable_ipv6() { ipv6_ = true; return *this; }
ServerBuilder& ServerBuilder::disable_ipv6() { ipv6_ = false; return *this; }

ServerBuilder& ServerBuilder::set_request_headers_size_limit(std::size_t limit) { headers_limit_ = limit; return *this; }
ServerBuilder& ServerBuilder::set_request_body_size_limit(std::size_t limit) { body_limit_ = limit; return *this; }
ServerBuilder& ServerBuilder::set_request_target_size_limit(std::size_t limit) { request_target_limit_ = limit; return *this; }

ServerBuilder& ServerBuilder::set_request_timeout(const timeval& timeout) { this->timeout_ = timeout; return *this; }

Server ServerBuilder::build() {
    if (!ipv4_ && !ipv6_) {
        throw std::runtime_error("At least one from ipv4 and ipv6 flags has to be enabled.");
    }
    if (!std::filesystem::is_directory(router_.public_dir_)) {
        throw std::runtime_error("The provided public_dir path is not a directory!");
    }
    return Server(*this);
}

Server::Server(const ServerBuilder& b)
    : parser_(b.headers_limit_, b.body_limit_, b.request_target_limit_),
    router_(b.router_),
    pool_(std::thread::hardware_concurrency()),
    timeout_(b.timeout_),
    is_running_(false)
{
    if (b.ipv4_ && !b.ipv6_) {
        ssock_ = ServerSocket(SocketAddr4(b.port_));
    } else if (!b.ipv4_ && b.ipv6_) {
        ssock_ = ServerSocket(SocketAddr6(b.port_));
    } else {
        ssock_ = ServerSocket(SocketAddr46(b.port_));
    }
}

void log_exception(std::exception_ptr eptr) {
    try {
        if (eptr) {
            std::rethrow_exception(eptr);
        }
    } catch (const std::exception& e) {
        logger::error("Exception: {}\n", e.what());
    } catch (...) {
        logger::error("Unknown exception type occurred.\n");
    }
}

void Server::handle_client(Connection&& conn)
{
    Request request;
    Response response;

    try {
        auto req = parser_.parse_request(conn);

        if (req.has_value()) {
            request = *req;
            response = router_.handle_request(request);
        } else if (req.error().code == StatusCode::None) {
            return; // client closed
        } else {
            response = Response(req.error());
        }
    } catch (...) {
        log_exception(std::current_exception());
        response = Response(ServerErr(StatusCode::InternalServerError));
    }

    logger::info("{} /{} -> {} {}",
        to_string(request.method),
        request.target.relative_path,
        static_cast<int>(response.code),
        to_string(response.code)
    );

    try {
        conn.send(response.to_string());
    } catch (...) {
        log_exception(std::current_exception());
    }
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
        pool_.submit_task([this, conn = Connection(std::move(csock))]() mutable {
            handle_client(std::move(conn));
        });
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

SocketAddr6 Server::get_addr6()
{
    return SocketAddr6(ssock_.srv_port_);
}
