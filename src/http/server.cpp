#include "http/server.hpp"
#include "logger.hpp"
#include "net/common.hpp"

#include <exception>
#include <stdexcept>
#include <string>
#include <thread>

using namespace http;
using namespace net;

/** @brief Sets the root directory for serving static files. */
ServerBuilder& ServerBuilder::set_public_dir(const std::filesystem::path& public_dir) { router_.public_dir_ = public_dir; return *this; }
/**
 * @brief Sets the HTML template used for auto-generating server error pages.
 *
 * @details This function allows you to customize the body of HTTP error responses
 *          (like 404 Not Found or 500 Internal Server Error). The provided string
 *          must be valid `text/html`.
 *
 *          Internally, this template is processed using `std::format`. Therefore,
 *          you must use specific placeholders to inject the error details dynamically.
 *
 * @note **Formatting Rules:**
 *       - Use `{0}` where the numeric HTTP Error Code should appear.
 *       - Use `{1}` where the HTTP Error Message should appear.
 *       - **CRITICAL:** Because `std::format` is used, any literal curly braces
 *         in your CSS or JavaScript must be doubled (e.g., use `{{` and `}}`)
 *         so they are rendered correctly and don't crash the formatter.
 *
 * @param template_content The raw HTML string containing the template.
 * @return A reference to the builder for method chaining.
 */
ServerBuilder& ServerBuilder::set_error_page_template(std::string&& err_template) { Response::error_template_ = std::move(err_template); return *this; }
/** @brief Enables auto-generation of HTML indexes for directory requests. */
ServerBuilder& ServerBuilder::enable_directory_listing() { router_.list_dir_ = true; return *this; }
/** @brief Disables auto-generation of HTML indexes for directory requests. */
ServerBuilder& ServerBuilder::disable_directory_listing() { router_.list_dir_ = false; return *this; }

/**
 * @brief Registers a custom callback handler for a specific dynamic route.
 *
 * @details Binds an HTTP method and a requested URI path to a custom handler function.
 *          When the server receives a matching request, this function will be executed.
 *
 * @warning **Path Formatting Requirements:**
 *          The caller is strictly responsible for providing the path in a **relative**
 *          and **normalized** format. The router performs exact string matching for speed,
 *          so invalid formatting will result in broken routes.
 *          - **Rule 1:** The path must NOT start with a leading slash `/`.
 *          - **Rule 2:** The path must be fully normalized (no redundant slashes or traversal dots).
 *
 *          @par Examples:
 *          - **Good:** `api/create-user`
 *          - **Bad:** `/api/create-user` (Leading slash)
 *          - **Bad:** `api/users/.././//.//create-user` (Not normalized)
 *
 * @param path The relative, normalized URI path to match.
 * @param method The required HTTP method (e.g., GET, POST) to trigger this route.
 * @param func The callable handler function to process the request.
 * @return A reference to the builder for method chaining.
 */
ServerBuilder& ServerBuilder::add_route(std::string&& path, http::RequestMethod method, handlers::Router::HandlerFunc&& func)
{
    router_.add_route(std::move(path), method, std::move(func));
    return *this;
}

/** @brief Sets the port the server will listen on. */
ServerBuilder& ServerBuilder::set_port(in_port_t port) { this->port_ = port; return *this; }

/** @brief Enables binding to IPv4 network interfaces. */
ServerBuilder& ServerBuilder::enable_ipv4() { ipv4_ = true; return *this; }
/** @brief Disables binding to IPv4 network interfaces. */
ServerBuilder& ServerBuilder::disable_ipv4() { ipv4_ = false; return *this; }

/** @brief Enables binding to IPv6 network interfaces. */
ServerBuilder& ServerBuilder::enable_ipv6() { ipv6_ = true; return *this; }
/** @brief Disables binding to IPv6 network interfaces. */
ServerBuilder& ServerBuilder::disable_ipv6() { ipv6_ = false; return *this; }

/** @brief Sets the maximum allowed byte size for HTTP headers. */
ServerBuilder& ServerBuilder::set_request_headers_size_limit(std::size_t limit) { headers_limit_ = limit; return *this; }
/** @brief Sets the maximum allowed byte size for HTTP bodies */
ServerBuilder& ServerBuilder::set_request_body_size_limit(std::size_t limit) { body_limit_ = limit; return *this; }
/** @brief Sets the maximum allowed byte size for URIs. */
ServerBuilder& ServerBuilder::set_request_target_size_limit(std::size_t limit) { request_target_limit_ = limit; return *this; }

/** @brief Sets the socket timeout duration to drop slow or dead clients. */
ServerBuilder& ServerBuilder::set_request_timeout(const timeval& timeout) { this->timeout_ = timeout; return *this; }

/** @brief Defines the number of worker threads in the concurrent execution pool. */
ServerBuilder& ServerBuilder::set_thread_count(std::size_t count) { thread_count_ = count; return *this; }

/**
 * @brief Validates the configuration and constructs the final Server instance.
 * @return A fully initialized, ready-to-run HTTP Server.
 */
Server ServerBuilder::build() {
    if (!ipv4_ && !ipv6_) {
        throw std::runtime_error("At least one from ipv4 and ipv6 flags has to be enabled.");
    }
    if (!std::filesystem::is_directory(router_.public_dir_)) {
        throw std::runtime_error("The provided public_dir path is not a directory!");
    }
    return Server(*this);
}

/**
 * @brief Private constructor. The Server can only be instantiated via a ServerBuilder.
 * @param builder The fully configured builder instance.
 */
Server::Server(const ServerBuilder& b)
    : parser_(b.headers_limit_, b.body_limit_, b.request_target_limit_),
    router_(b.router_),
    pool_(b.thread_count_),
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
/**
 * @brief The core execution block for a single client connection.
 *
 * @details This function is submitted to the thread pool. It parses the incoming
 *          HTTP request, passes it to the router, and sends the generated response
 *          back over the network.
 *
 * @param conn The active, buffered client connection.
 */
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
        method::to_string(request.method),
        request.target.relative_path,
        static_cast<int>(response.code),
        to_string(response.code)
    );

    try {
        conn.send(response.to_string());
        logger::debug("Response sent :)");
    } catch (...) {
        log_exception(std::current_exception());
    }
}

/**
 * @brief Starts the main server event loop.
 * @details Blocks the current thread, continuously polling the server socket 
 *          and dispatching accepted connections to the thread pool.
 */
void Server::run()
{
    is_running_ = true;
    while (is_running_) {
        SocketAddr6 client_addr;
        ClientSocket csock = ssock_.poll(client_addr, &timeout_);
        if (!csock.is_valid()) {
            continue;
        }
        bool success = pool_.submit_task([this, conn = Connection(std::move(csock))]() mutable {
            handle_client(std::move(conn));
        });
        if (!success) {
            logger::error("Trying to submit new task to a closed queue! Stoping the server ...");
            is_running_ = false;
        }
    }
}

/**
 * @brief Gracefully signals the server to halt the event loop.
 */
void Server::stop()
{
    is_running_ = false;
}

/** @brief Retrieves the actual IPv4 address and port the server is bound to. */
SocketAddr4 Server::get_addr()
{
    return SocketAddr4(std::string(net::kLocalhostIpv4), ssock_.srv_port_);
}

/** @brief Retrieves the actual IPv6 address and port the server is bound to. */
SocketAddr6 Server::get_addr6()
{
    return SocketAddr6(std::string(net::kLocalhostIpv6), ssock_.srv_port_);
}
