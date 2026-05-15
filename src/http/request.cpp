#include "http/request.hpp"

#include <string>
#include <vector>
#include <sstream>

namespace http {

namespace method {
    /**
     * @brief Converts a raw string view from the network into a strongly-typed enum.
     * @param str The parsed HTTP method string.
     * @return The corresponding `RequestMethod`, or `RequestMethod::None` if unrecognized.
     */
    RequestMethod from_string(std::string_view str) {
        if (str == kGet)    return RequestMethod::Get;
        if (str == kPost)   return RequestMethod::Post;
        if (str == kPut)    return RequestMethod::Put;
        if (str == kDelete) return RequestMethod::Delete;
        return RequestMethod::None;
    }

    /**
     * @brief Converts a strongly-typed enum back into its string representation.
     * @param method The internal method enum.
     * @return The exact uppercase string required by the HTTP standard.
     */
    std::string to_string(RequestMethod method) {
        switch (method) {
            case RequestMethod::Get:    return std::string(kGet);
            case RequestMethod::Post:   return std::string(kPost);
            case RequestMethod::Put:    return std::string(kPut);
            case RequestMethod::Delete: return std::string(kDelete);
            default:                    return "<?METHOD?>";
        }
    }
} // end of `method` namespace

static std::string url_decode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length()); // Prevent unnecessary memory allocations

    for (std::size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            decoded += (char)std::stoi(encoded.substr(i+1, 2), nullptr, 16);
            i += 2;
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
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

/// @brief Default constructor. Creates an empty target.
RequestTarget::RequestTarget()
    : relative_path("")
{}

RequestTarget::RequestTarget(std::string&& rel_path)
    : relative_path(rel_path)
{}

/**
 * @brief Factory method to safely parse a raw URI string.
 *
 * @details Validates the target string and extracts the routing path.
 *
 * @param raw_target The raw target string from the Request-Line.
 * @return A valid `RequestTarget` object, or an error string if parsing fails.
 */
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

    std::string relative_path;
    try {
        relative_path = url_decode(normalize_path(std::string(buf)));
    } catch (const std::exception&) {
        return std::unexpected("Couldn't decode escape sequences in request target: " + std::string(raw_target));
    }

    return RequestTarget(std::move(relative_path));
}

} // end of `http` namespace
