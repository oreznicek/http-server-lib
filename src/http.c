#include "http.hpp"

http::RequestMethod to_http_request_method(std::string_view str) {
    switch (str) {
        case "GET":     return HttpRequestMethod::GET;
        case "POST":    return HttpRequestMethod::POST;
        case "PUT":     return HttpRequestMethod::PUT;
        case "DELETE":  return HttpRequestMethod::DELETE;
        default:
            return HttpRequestMethod::NONE;
    }
}
