#include "http/connection.hpp"
#include "logger.hpp"

using namespace http;

/**
 * @brief Internal helper to pull the next chunk of data from the socket.
 * @param buffer The string to append the newly read data into.
 * @return `StatusCode::Ok` on success, or an appropriate HTTP error status on failure.
 */
StatusCode Connection::read_chunk(std::string& buffer)
{
    buffer.resize(buffer.size() + kChunk);
    int bytes = csock_.recv(&buffer[buffer.size() - kChunk], kChunk);
    buffer.resize(buffer.size() - kChunk + std::max(0, bytes));
    logger::debug("Connection.recv() -> {}", bytes);

    if (bytes < 0) {
        // TODO: Add logging of the errors
        logger::warning("Connection.recv() failed with: {}", SOCK_ERROR_CODE);
#ifdef _WIN32
        switch (WSAGetLastError()) {
            case WSAEINTR:
                return read_chunk(buffer);
            case WSAETIMEDOUT:
            case WSAEWOULDBLOCK:
                return StatusCode::RequestTimeout;
            case WSAECONNRESET:
            case WSAECONNABORTED:
                return StatusCode::None;
            case WSAENOTSOCK:
            case WSAENOTCONN:
            case WSAEINVAL:
            case WSAEOPNOTSUPP:
            case WSAEBADF:
                return StatusCode::InternalServerError;
            default:
                return StatusCode::None;
        }
#else
        switch (errno) {
            case EINTR:
                return read_chunk(buffer);
            case EAGAIN:
    #if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK:
    #endif
                return StatusCode::RequestTimeout;
            case ECONNRESET:
            case ETIMEDOUT:
                return StatusCode::None;
            case EBADF:
            case ENOTSOCK:
            case ENOTCONN:
            case EINVAL:
            case EOPNOTSUPP:
                return StatusCode::InternalServerError;
            default: // EIO, ENOBUFS, ENOMEM
                return StatusCode::None;
        }
#endif
    }  else if (bytes == 0) {
        logger::warning("Connection.recv() received TCP FIN");
        if (buffer.empty()) {// TCP FIN
            return StatusCode::None;
        }
    }
    return StatusCode::Ok;
}

Connection::Connection()
{}

/**
 * @brief Constructs a connection by taking ownership of a connected client socket.
 * @param csock The connected client socket (moved).
 */
Connection::Connection(net::ClientSocket&& csock)
    : csock_(std::move(csock))
{}

/**
 * @brief Reads from the socket until a specific string delimiter is found.
 *
 * @details Continuously pulls data from the socket until the `delimiter`
 *          is encountered. Any excess bytes read past the delimiter are safely
 *          stored in the internal `leftover_` buffer for the next read call.
 *
 * @param delimiter The string sequence to search for (e.g., "\r\n\r\n" for HTTP headers).
 * @param max_bytes The maximum allowable bytes to read before aborting (to prevent memory exhaustion).
 * @return An `std::expected` containing the parsed string up to the delimiter, 
 *         or an HTTP `StatusCode` if a network error occurs or `max_bytes` is exceeded.
 */
std::expected<std::string, StatusCode> Connection::read_until(std::string_view delimiter, std::size_t max_bytes)
{
    std::string buffer;
    size_t bytes_to_move = std::min(leftover_.size(), max_bytes);
    buffer.append(leftover_, 0, bytes_to_move);
    leftover_.erase(0, bytes_to_move);

    while (true) {
        // TODO: Optimization - Can just search for the delimiter in the last two chunks
        std::size_t i = buffer.find(delimiter);
        if (i != std::string::npos) {
            leftover_.insert(0, buffer.substr(i + delimiter.size()));
            buffer.resize(i);
            if (buffer.size() > max_bytes) {
                return std::unexpected(StatusCode::ContentTooLarge);
            }
            break;
        }

        if (buffer.size() > max_bytes) {
            return std::unexpected(StatusCode::ContentTooLarge);
        }

        StatusCode code = read_chunk(buffer);
        if (code != StatusCode::Ok) {
            return std::unexpected(code);
        }
    }

    return buffer;
}

/**
 * @brief Reads an exact amount of bytes from the connection.
 *
 * @details Drains the internal `leftover_` buffer first, and if more data is needed,
 *          blocks and reads directly from the socket until exactly `bytes` are retrieved.
 *
 * @param bytes The exact number of bytes to read (usually derived from the Content-Length header).
 * @return An `std::expected` containing the data string, or an HTTP `StatusCode` on network failure.
 */
std::expected<std::string, StatusCode> Connection::read(std::size_t bytes)
{
    std::string buffer = std::move(leftover_);

    while (true) {
        if (buffer.size() >= bytes) {
            leftover_ = buffer.substr(bytes);
            buffer.resize(bytes);
            break;
        }
        StatusCode code = read_chunk(buffer);
        if (code != StatusCode::Ok) {
            return std::unexpected(code);
        }
    }

    return buffer;
}

/**
 * @brief Transmits data over the network connection.
 *
 * @param buffer The payload string to send.
 * @return true if the buffer was sent successfully, false otherwise.
 */
bool Connection::send(const std::string& buffer)
{
    return csock_.send(buffer);
}
