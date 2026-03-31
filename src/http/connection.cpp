#include "http/connection.hpp"

#include <cstring>
#include <format>

using namespace http;

Connection::Connection(net::ClientSocket&& csock)
    : csock_(std::move(csock))
{}

Connection::ReadResult Connection::read_until(std::string_view delimiter, std::size_t max_bytes)
{
    static constexpr int CHUNK = 1024;

    ReadResult result = {
        .status = StatusCode::Ok,
        .data = leftover_
    };
    std::string& buffer = result.data;
    int bytes;

    do {
        buffer.resize(buffer.size() + CHUNK);
        bytes = csock_.recv(&buffer[buffer.size() - CHUNK], CHUNK);

        if (bytes < 0) {
            // TODO: Add logging of the errors
            switch (errno) {
                case EINTR:
                    buffer.resize(buffer.size() - CHUNK);
                    continue; // interrupt happened -> call recv again
                case EAGAIN:
            #if EAGAIN != EWOULDBLOCK
                case EWOULDBLOCK:
            #endif
                    result.status = StatusCode::RequestTimeout;
                    break;
                case ECONNRESET:
                case ETIMEDOUT:
                    result.status = StatusCode::None;
                    break;
                case EBADF:
                case ENOTSOCK:
                case ENOTCONN:
                case EINVAL:
                case EOPNOTSUPP:
                    result.status = StatusCode::InternalServerError;
                    break;
                default: // EIO, ENOBUFS, ENOMEM
                    result.status = StatusCode::None;
                    break;
            }
            break;
        }  else if (bytes == 0) { // TCP FIN
            result.status = StatusCode::None;
            break;
        } else if (bytes < CHUNK) {
            buffer.resize(buffer.size() - CHUNK + bytes);
        }

        if (delimiter == "") {
            continue;
        }

        // TODO: Optimization - Can just search for the delimiter in the last two chunks
        std::size_t i = buffer.find(delimiter);
        if (i != std::string::npos) {
            leftover_ = buffer.substr(i + delimiter.size());
            buffer.resize(i);
            return result;
        }
    } while (buffer.size() <= max_bytes);

    result.status = StatusCode::ContentTooLarge;
    return result;
}


bool Connection::send(const std::string& buffer)
{
    return csock_.send(buffer);
}
