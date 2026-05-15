#include "http/mime.hpp"

namespace http::mime {

/**
 * @brief Converts a strongly-typed MIME enum into its standard HTTP string representation.
 *
 * @param type The internal `Type` enum value.
 * @return The exact string required for the HTTP "Content-Type" header (e.g., "application/json").
 */
std::string to_string(Type type)
{
    switch (type) {
        case Type::TextHtml: return "text/html";
        case Type::TextJavascript: return "text/javascript";
        case Type::TextPlain: return "text/plain";
        case Type::TextCss: return "text/css";
        case Type::ApplicationJson: return "application/json";
        case Type::ImagePng: return "image/png";
        case Type::ImageJpeg: return "image/jpeg";
        default: return "application/octet-stream";
    }
}

/**
 * @brief Determines the appropriate MIME type based on a file extension.
 *
 * @details Used primarily by the static file server to dynamically deduce
 *          what type of file is being read from disk so the browser knows
 *          how to render it.
 *
 * @param ext The file extension string, typically including the dot (e.g., ".css", ".html").
 * @return The corresponding `Type` enum. If the extension is unrecognized,
 *         it safely defaults to `Type::OctetStream`.
 */
Type extension_to_type(std::string_view ext)
{
    if (ext == ".html" || ext == ".htm") return Type::TextHtml;
    if (ext == ".js") return Type::TextJavascript;
    if (ext == ".txt") return Type::TextPlain;
    if (ext == ".css") return Type::TextCss;
    if (ext == ".json") return Type::ApplicationJson;
    if (ext == ".png") return Type::ImagePng;
    if (ext == ".jpg" || ext == ".jpeg") return Type::ImageJpeg;
    return Type::OctetStream;
}

} // end of `http::mime` namespace
