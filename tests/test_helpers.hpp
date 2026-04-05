#ifndef _TEST_HELPERS_HPP
#define _TEST_HELPERS_HPP

#include <http/http.hpp>
#include <http/server.hpp>

#include "response_parser.hpp"

#include <string>

void print_http_request(const std::string& str);
http::Response send_request_get_response(http::Server& srv, const std::string& request);
bool test_status_code_eq(http::Server& srv, const std::string& request, http::StatusCode expected);
bool test_status_code_diff(http::Server& srv, const std::string& request, http::StatusCode diff);

#endif
