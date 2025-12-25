#include "HttpRequestParser.hpp"

#include <errno.h>

std::string http::RequestParser::read_headers(ClientSocket** csock) {
    std::string buffer;
    int bytes;

    do {
        buffer.resize(buffer.size() + CHUNK);
        bytes = (*csock)->read(&buffer[buffer.size() - CHUNK], CHUNK);

        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // timeout -> send 408 and close connection with client
                (*csock)->close();
                *csock = nullptr;
            }
            perror("read");
            throw std::runtime_error("read() failed");
        }  else if (bytes == 0) { // client closed connection
            if (buffer.find(HEADERS_END) == std::string::npos) {
                (*csock)->close();
                *csock = nullptr;
            }
            break;
        } else if (bytes < CHUNK) {
            buffer.resize(buffer.size() - CHUNK + bytes);
        }

        if (buffer.size() > headers_limit) {
            // send 431
        }

    } while (buffer.find(HEADERS_END) == std::string::npos);

    return buffer;
}

http::RequestParser::read_body(ClientSocket** csock) {
    if (*csock == nullptr) { // Connection was already closed in `read_headers` either by us or client
        return "";
    }
}

HttpRequestParser::HttpRequestParser(std::size_t headers_limit, std::size_t body_limit)
    : headers_limit(headers_limit), body_limit(body_limit)
{}

HttpRequest HttpRequestParser::parse_request(ClientSocket* socket) {

}
