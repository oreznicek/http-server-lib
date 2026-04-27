#include "http/mime.hpp"

namespace http::mime {

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
