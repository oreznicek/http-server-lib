#include "http/connection.hpp"

#include <cstring>

using namespace http;

Connection::Connection(net::ClientSocket&& csock)
    : csock_(std::move(csock))
{}

Connection::ReadResult::ReadResult(StatusCode status)
    : status(status)
{}
Connection::ReadResult::ReadResult(StatusCode status, std::string&& data)
    : status(status), data(std::move(data))
{}

Connection::ReadResult Connection::read_until(std::string_view delimiter, std::size_t max_bytes)
{
    static constexpr int CHUNK = 1024;

    std::string buffer;
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
                    return Connection::ReadResult(StatusCode::RequestTimeout);
                case ECONNRESET:
                case ETIMEDOUT:
                    return Connection::ReadResult(StatusCode::None);
                case EBADF:
                case ENOTSOCK:
                case ENOTCONN:
                case EINVAL:
                case EOPNOTSUPP:
                    return Connection::ReadResult(StatusCode::InternalServerError);
                default: // EIO, ENOBUFS, ENOMEM
                    return Connection::ReadResult(StatusCode::None);
            }
        }  else if (bytes == 0) { // TCP FIN
            return Connection::ReadResult(StatusCode::None);
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
            return Connection::ReadResult(StatusCode::Ok, std::move(buffer));
        }
    } while (buffer.size() <= max_bytes);

    return Connection::ReadResult(StatusCode::ContentTooLarge);
}


bool Connection::send(const std::string& buffer)
{
    return csock_.send(buffer);
}
