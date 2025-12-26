#include "http/request_parser.hpp"

#include <stdexcept>
#include <errno.h>
#include <iostream>
#include <cctype>
#include <cstdlib>

using namespace http;
using namespace net;

std::string RequestParser::read_headers(net::ClientSocket** csock) {
    std::string buffer;
    int bytes;

    do {
        buffer.resize(buffer.size() + CHUNK);
        bytes = (*csock)->read(&buffer[buffer.size() - CHUNK], CHUNK);

        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                send_error_response(**csock, StatusCode::REQUEST_TIMEOUT);
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
            send_error_response(**csock, StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);
        }

    } while (buffer.find(HEADERS_END) == std::string::npos);

    return buffer;
}

RequestHeader RequestParser::parse_header(std::string_view line) {
    std::size_t i = line.find(':');
    std::string_view key = line.substr(0, i);
    std::string_view value = line.substr(i + 1);

    while (!value.empty() && std::isspace(value.front())) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(value.back())) {
        value.remove_suffix(1);
    }

    return RequestHeader{std::string(key), std::string(value)};
}

Request RequestParser::parse_headers(std::string_view headers) {
    Request req;

    std::cout << "/*" << headers << "*/" << std::endl;

    std::size_t i = headers.find(' ');
    req.method = to_request_method(headers.substr(0, i));
    headers = headers.substr(i + 1);

    i = headers.find(' ');
    req.relative_path = headers.substr(0, i);
    headers = headers.substr(i + 1);

    headers = headers.substr(headers.find("\r\n") + 2);

    std::string_view line;

    while ((i = headers.find("\r\n")) != std::string_view::npos)  {
        RequestHeader header = parse_header(headers.substr(0, i));

        if (header.key == "Host") {
            req.host = true;
        } else if (header.key == "Connection") {
            if (header.value == "keep-alive") {
                req.keep_alive = true;
            } else if (header.value == "close") {
                req.close = true;
            }
        } else if (header.key == "Content-Length") {
            // TODO: What if it is 0 or negative?
            req.content_length = std::stoi(header.value);
        }

        headers = headers.substr(i + 2);
    }

    return req;
}

std::string RequestParser::read_body(net::ClientSocket** csock) {
    if (*csock == nullptr) { // Connection was already closed in `read_headers` either by us or client
        return "";
    }
    return "";
}

RequestParser::RequestParser(std::size_t headers_limit, std::size_t body_limit)
    : headers_limit(headers_limit), body_limit(body_limit)
{}

Request RequestParser::parse_request(net::ClientSocket* socket) {
    Request req = parse_headers(read_headers(&socket));
    req.body = read_body(&socket);
    return req;
}

void http::send_error_response(ClientSocket& csock, StatusCode code) {
    send_error_response(csock, code, to_string(code));
}

void http::send_error_response(ClientSocket& csock, StatusCode code, std::string&& message) {
    std::string full_message = std::to_string((int)code) + " " + message;
    std::string resp_body = "<html><body><h1>" + full_message + "</h1></body></html>";
    std::string response =
        "HTTP/1.1 " + full_message + "\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(resp_body.size()) + "\r\n"
        "\r\n" + resp_body;
    csock.write(response.c_str(), response.size());
}
