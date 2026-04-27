#include "http/request.hpp"

#include <vector>
#include <sstream>

using namespace http;

RequestMethod http::to_request_method(std::string_view str)
{
    if (str == "GET") return RequestMethod::Get;
    else if (str == "POST") return RequestMethod::Post;
    else if (str == "PUT") return RequestMethod::Put;
    else if (str == "DELETE") return RequestMethod::Delete;
    return RequestMethod::None;
}

static std::string normalize_path(const std::string& path)
{
    std::vector<std::string> parts;
    std::string token;
    std::istringstream token_stream(path);

    while (std::getline(token_stream, token, '/')) {
        if (token == "" || token == ".") {
            continue;
        } else if (token == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        } else {
            parts.push_back(token);
        }
    }

    std::string normalized;
    if (!parts.empty()) {
        normalized += parts.front();
    }
    for (std::size_t i = 1; i < parts.size(); ++i) {
        normalized += "/" + parts[i];
    }

    return normalized;
}

RequestTarget::RequestTarget()
    : relative_path("")
{}

RequestTarget::RequestTarget(std::string&& relative_path)
    : relative_path(normalize_path(relative_path))
{}

std::expected<RequestTarget, std::string> RequestTarget::from(std::string_view raw_target)
{
    static constexpr std::string_view kScheme = "http:";
    static constexpr std::string_view kAuthorityDelimiter = "//";

    if (raw_target.empty()) {
        return std::unexpected("Missing Request Target");
    }

    std::string_view buf = raw_target;
    bool is_absolute_form = buf.starts_with(kScheme);

    if (is_absolute_form) {
        buf = buf.substr(kScheme.size());
        if (buf.starts_with(kAuthorityDelimiter)) {
            buf = buf.substr(kAuthorityDelimiter.size());
        } else {
            return std::unexpected("Missing " + std::string(kAuthorityDelimiter) + " in absolute-form request target: " + std::string(raw_target));
        }
    }

    std::size_t i = buf.find('/');
    if (i == 0 && is_absolute_form) {
        return std::unexpected("Missing authority part in request target: " + std::string(raw_target));
    } else if (i == std::string_view::npos) {
        return RequestTarget();
    } else if (is_absolute_form) {
        buf = buf.substr(i + 1);
    }

    return RequestTarget(std::string(buf));
}
