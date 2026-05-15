#ifndef _HTTP_MIME_HPP
#define _HTTP_MIME_HPP

#include <string>

namespace http::mime {

/// @brief Represents the standard HTTP Media Types supported by the server.
enum class Type {
    TextHtml,
    TextJavascript,
    TextPlain,
    TextCss,
    ApplicationJson,
    ImagePng,
    ImageJpeg,
    OctetStream
};

std::string to_string(Type type);
Type extension_to_type(std::string_view ext);

} // end of `http::mime` namespace

#endif
