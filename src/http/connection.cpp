#include "http/connection.hpp"

using namespace http;

StatusCode Connection::read_chunk(std::string& buffer)
{
    buffer.resize(buffer.size() + CHUNK);
    int bytes = csock_.recv(&buffer[buffer.size() - CHUNK], CHUNK);
    buffer.resize(buffer.size() - CHUNK + std::max(0, bytes));

    if (bytes < 0) {
        // TODO: Add logging of the errors
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
    }  else if (bytes == 0) {
        if (buffer.empty()) {// TCP FIN
            return StatusCode::None;
        }
    }
    return StatusCode::Ok;
}

Connection::Connection(net::ClientSocket&& csock)
    : csock_(std::move(csock))
{}

std::expected<std::string, StatusCode> Connection::read_until(std::string_view delimiter, std::size_t max_bytes)
{
    std::string buffer = std::move(leftover_);

    while (true) {
        if (buffer.size() > max_bytes) {
            return std::unexpected(StatusCode::ContentTooLarge);
        }
        // TODO: Optimization - Can just search for the delimiter in the last two chunks
        std::size_t i = buffer.find(delimiter);
        if (i != std::string::npos) {
            leftover_ = buffer.substr(i + delimiter.size());
            buffer.resize(i);
            break;
        }
        StatusCode code = read_chunk(buffer);
        if (code != StatusCode::Ok) {
            return std::unexpected(code);
        }
    }

    return buffer;
}

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

bool Connection::send(const std::string& buffer)
{
    return csock_.send(buffer);
}
