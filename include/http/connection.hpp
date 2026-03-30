#ifndef _HTTP_CONNECTION_HPP
#define _HTTP_CONNECTION_HPP

#include "http/http.hpp"
#include "net/socket.hpp"

#include <string>
#include <string_view>

namespace http {

class Connection {
    net::ClientSocket csock_;
    std::string leftover_;
public:
    Connection(net::ClientSocket&& csock);
    struct ReadResult {
        StatusCode status;
        std::string data;
    };
    ReadResult read_until(std::string_view delimiter, std::size_t max_bytes);
};

} // end of `http` namespace

#endif
