#include "http/http.hpp"

using namespace http;

RequestMethod http::to_request_method(std::string_view str) {
    if (str == "GET") return RequestMethod::Get;
    else if (str == "POST") return RequestMethod::Post;
    else if (str == "PUT") return RequestMethod::Put;
    else if (str == "DELETE") return RequestMethod::Delete;
    return RequestMethod::None;
}

std::string http::to_string(StatusCode code) {
    switch (code) {
        case StatusCode::Ok:
            return "OK";
        case StatusCode::BadRequest:
            return "Bad Request";
        case StatusCode::RequestTimeout:
            return "Request Timeout";
        case StatusCode::ContentTooLarge:
            return "Content Too Large";
        case StatusCode::UriTooLong:
            return "URI Too Long";
        case StatusCode::RequestHeaderFieldsTooLarge:
            return "Request Header Field Too Large";
        case StatusCode::InternalServerError:
            return "Internal Server Error";
        case StatusCode::NotImplemented:
            return "Not Implemented";
        case StatusCode::HttpVersionNotSupported:
            return "HTTP Version Not Supported";
        case StatusCode::None:
            return "";
    }
}
