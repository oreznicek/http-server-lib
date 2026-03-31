#ifndef _TEST_RESPONSE_PARSER_HPP
#define _TEST_RESPONSE_PARSER_HPP

#include <http/http.hpp>
#include <http/connection.hpp>
#include <net/socket.hpp>

#include <expected>

namespace http {

struct Response {
    StatusCode code;
    bool valid;
    Response(); // invalid response
    Response(StatusCode code); // valid response
};

class ResponseParser {
public:
    Response parse_response(Connection& conn) noexcept;
};

} // end of `http` namespace

#endif
