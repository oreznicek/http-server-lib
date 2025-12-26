#include "http/http.hpp"

using namespace http;

RequestMethod http::to_request_method(std::string_view str) {
    if (str == "GET") return RequestMethod::GET;
    else if (str == "POST") return RequestMethod::POST;
    else if (str == "PUT") return RequestMethod::PUT;
    else if (str == "DELETE") return RequestMethod::DELETE;
    return RequestMethod::NONE;
}

std::string http::to_string(StatusCode code) {
    switch (code) {
        case StatusCode::BAD_REQUEST:
            return "Bad Request";
        case StatusCode::REQUEST_TIMEOUT:
            return "Request timeout";
        case StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE:
            return "Request header field too large";
    }
    return "";
}
