#include "response_parser.hpp"
#include "http/server.hpp"

#include <string>
#include <string_view>

using namespace http;

Response::Response()
    : code(StatusCode::None), valid(false)
{}

Response::Response(StatusCode code)
    : code(code), valid(true)
{}

Response ResponseParser::parse_response(Connection& conn) noexcept
{
    auto res = conn.read_until("\r\n", 500);
    if (!res.has_value()) {
        return Response();
    }

    std::string_view status_line = *res;
    std::size_t i = status_line.find(' ');
    if (status_line.substr(0, i) != kServerHttpVersion) {
        return Response();
    }
    status_line = status_line.substr(i + 1);

    i = status_line.find(' ');
    try {
        int code = std::stoi(std::string(status_line.substr(0, i)));
        return Response(static_cast<StatusCode>(code));
    } catch (...) {
        return Response();
    }
}
