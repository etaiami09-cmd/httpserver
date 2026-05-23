#ifndef HTTP_HTTP_HPP
#define HTTP_HTTP_HPP

#include "Request.hpp"
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <chrono>
#include <sys/types.h>
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <sstream>

enum class HTTPMethod {
    Get,
    Post,
    Put,
    Patch,
    Delete,
    Invalid
};

inline HTTPMethod getMethod(const std::string& request) {
    if (request.starts_with("GET ")) {
        return HTTPMethod::Get;
    }
    if (request.starts_with("POST ")) {
        return HTTPMethod::Post;
    }
    if (request.starts_with("PUT ")) {
        return HTTPMethod::Put;
    }
    if (request.starts_with("PATCH ")) {
        return HTTPMethod::Patch;
    }
    if (request.starts_with("DELETE ")) {
        return HTTPMethod::Delete;
    }
    return HTTPMethod::Invalid;
}

inline std::string getHTTPDate() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%a, %d %b %Y %H:%M:%S GMT}", 
                       std::chrono::floor<std::chrono::seconds>(now));
}

inline std::string getPath(const std::string& request) {
    // only call if getMethod(request) != HTTPMethod::Invalid
    size_t start = request.find_first_of(' ') + 1;
    size_t end = request.find_first_of(' ', start + 1);
    return request.substr(start, end-start);
}

inline std::string getHTTPVersion(const std::string& request) {
    size_t firstSpace = request.find_first_of(' ');
    size_t secondSpace = request.find_first_of(' ', firstSpace + 1);
    size_t newline = request.find("\r\n");
    return std::string(request.begin() + secondSpace + 1, request.begin() + newline);
}

using ParameterList = std::optional<std::unordered_map<std::string, std::string>>;
using ParameterListInit = std::unordered_map<std::string, std::string>;

inline size_t countChars(const std::string& str, char c) {
    size_t count = 0;
    for (char i : str) {
        if (i == c) count++;
    }
    return count;
}

inline void parsePath(ParameterList& params, std::string& path, const std::string& request) {
    size_t firstSpace = request.find_first_of(' ');
    size_t secondSpace = request.find_first_of(' ', firstSpace + 1);
    std::string arguments = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    if (!arguments.contains('?')) {
        params = std::nullopt;
        path = arguments;
    }
    else {
        size_t paramStart = arguments.find_first_of('?');
        path = arguments.substr(0, paramStart);
        std::string rawParams = arguments.substr(paramStart + 1);
        params = ParameterList(ParameterListInit{countChars(rawParams, '=')});
        while (rawParams.size() != 0) {
            size_t delim = rawParams.find_first_of('=');
            std::string key(rawParams.begin(), rawParams.begin() + delim);
            size_t valueEnd = rawParams.find_first_of('&');
            std::string value;
            if (valueEnd == std::string::npos) {
                value = rawParams.substr(delim + 1);
            }
            else {
                value = std::string(rawParams.begin() + delim + 1, rawParams.begin() + valueEnd);
            }
            params.value()[key] = value;
            rawParams = rawParams.substr((valueEnd == std::string::npos)
                                            ? rawParams.size() : valueEnd + 1);
        }
    }
}

enum class HTTPHeader {
    ContentType,
    ContentLength,
    ConnectionClose,
    Host,
    Date,
    Cookie,
    SetCookie
};

enum class HTTPContentType {
    ApplicationJson,
    TextHtml,
    TextPlain
};

template<>
struct std::formatter<HTTPMethod> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const HTTPMethod& p, std::format_context& ctx) const {
        switch (p) {
            case (HTTPMethod::Get): return std::format_to(ctx.out(), "GET");
            case (HTTPMethod::Post): return std::format_to(ctx.out(), "POST");
            case (HTTPMethod::Put): return std::format_to(ctx.out(), "PUT");
            case (HTTPMethod::Patch): return std::format_to(ctx.out(), "PATCH");
            case (HTTPMethod::Delete): return std::format_to(ctx.out(), "DELETE");
            default: return std::format_to(ctx.out(), "INVALID");
        }
    }
};

template<>
struct std::formatter<HTTPHeader> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const HTTPHeader& p, std::format_context& ctx) const {
        switch (p) {
            case (HTTPHeader::ContentType): return std::format_to(ctx.out(), "Content-Type");
            case (HTTPHeader::ContentLength): return std::format_to(ctx.out(), "Content-Length");
            case (HTTPHeader::ConnectionClose): return std::format_to(ctx.out(), "Connection: close");
            case (HTTPHeader::Host): return std::format_to(ctx.out(), "Host");
            case (HTTPHeader::Date): return std::format_to(ctx.out(), "Date");
            case (HTTPHeader::Cookie): return std::format_to(ctx.out(), "Cookie");
            case (HTTPHeader::SetCookie): return std::format_to(ctx.out(), "Set-Cookie");
        }
    }
};

template<>
struct std::formatter<HTTPContentType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const HTTPContentType& p, std::format_context& ctx) const {
        switch (p) {
            case (HTTPContentType::ApplicationJson):
                return std::format_to(ctx.out(), "application/json");
            case (HTTPContentType::TextHtml):
                return std::format_to(ctx.out(), "text/html; charset=UTF-8");
            case (HTTPContentType::TextPlain):
                return std::format_to(ctx.out(), "text/plain; charset=UTF-8");
            default:
                return std::format_to(ctx.out(), "**invalid_content_type**");
        }
    }
};

namespace CookieLength {
    inline uint64_t Seconds(uint64_t base) {return base;}
    inline uint64_t Minutes(uint64_t base) {return base * 60;}
    inline uint64_t Hours(uint64_t base) {return Minutes(base) * 60;}
    inline uint64_t Days(uint64_t base) {return Hours(base) * 24;}
    inline uint64_t Weeks(uint64_t base) {return Days(base) * 7;}
    inline uint64_t Months(uint64_t base) {return Weeks(base) * 4;}
    inline uint64_t Years(uint64_t base) {return Days(base) * 365.25;}
} // namespace CookieLength

struct ResponseCookie {
    std::string key;
    std::string value;
    uint64_t maxAge = 3600;

    ResponseCookie(const std::string& key, const std::string& value)
        : key(key), value(value) {}

    ResponseCookie& addAge(uint64_t age) {
        maxAge = age;
        return *this;
    }
};

template<HTTPHeader header>
using HeaderValueType = std::conditional_t<header == HTTPHeader::ContentType, HTTPContentType,
                        std::conditional_t<header == HTTPHeader::ContentLength, size_t,
                        std::conditional_t<header == HTTPHeader::ConnectionClose, std::nullptr_t,
                        std::conditional_t<header == HTTPHeader::Host, std::string,
                        std::conditional_t<header == HTTPHeader::Date, std::string,
                        std::conditional_t<header == HTTPHeader::Cookie, std::unordered_map<std::string, std::string>,
                        std::conditional_t<header == HTTPHeader::SetCookie, ResponseCookie,
std::nullptr_t>>>>>>>;

template<HTTPHeader header>
using PotentialHeader = std::optional<HeaderValueType<header>>;

inline HeaderValueType<HTTPHeader::Host> parseHost(const std::string& request) {
    size_t hostStart = request.find("\r\nHost: ") + 8;
    size_t hostEnd = request.find("\r\n", hostStart);
    return std::string(request.begin() + hostStart, request.begin() + hostEnd);
}

inline PotentialHeader<HTTPHeader::Cookie> parseCookies(const std::string& request) {
    std::string_view parseView(request.begin(), request.begin() + request.find("\r\n\r\n") + sizeof("\r\n\r\n"));
    std::unordered_map<std::string, std::string> cookies;
    size_t pviewsize = parseView.size();
    size_t ssize = request.size();
    size_t cookiesStart = parseView.find("\r\nCookie: ");
    size_t cookiesEnd = parseView.find("\r\n", cookiesStart + 1);
    parseView = {parseView.begin() + cookiesStart + sizeof("\r\nCookie: "),
        parseView.begin() + cookiesEnd};
    while (!parseView.empty()) {
        size_t delim = parseView.find('=');
        size_t end = parseView.find(';', delim);
        std::string key{parseView.begin(), parseView.begin() + delim};
        std::string value;
        if (end != std::string::npos) {
            value = {parseView.begin() + delim + 1, parseView.begin() + end};
            parseView = {parseView.begin() + end + 2, parseView.end()};
        }
        else {
            value = {parseView.begin() + delim + 1, parseView.end()};
            parseView = {parseView.end(), parseView.end()};
        }
        cookies[key] = value;
    }
    return cookies;
}

struct GetRequest {
    ParameterList params;
    std::string path;
    HeaderValueType<HTTPHeader::Host> host;
    PotentialHeader<HTTPHeader::Cookie> cookies;

    GetRequest(const std::string& request) {
        parsePath(params, path, request);
        cookies = parseCookies(request);
        host = parseHost(request);
    }
};

struct DeleteRequest {
    ParameterList params;
    std::string path;
    HeaderValueType<HTTPHeader::Host> host;
    PotentialHeader<HTTPHeader::Cookie> cookies;

    DeleteRequest(const std::string& request) {
        parsePath(params, path, request);
        cookies = parseCookies(request);
        host = parseHost(request);
    }
};

inline PotentialHeader<HTTPHeader::ContentType> parseContentType(const std::string& request) {
    size_t pos = request.find("\r\nContent-Type: ");
    if (pos == std::string::npos) return {};
    size_t typeStart = pos + 16;
    size_t typeEnd = request.find("\r\n", typeStart);
    std::string rawType(request.begin() + typeStart, request.begin() + typeEnd);
    if (rawType == "application/json") return HTTPContentType::ApplicationJson;
    if (rawType == "text/plain") return HTTPContentType::TextPlain;
    if (rawType == "text/html") return HTTPContentType::TextHtml;
    return {};
}

inline PotentialHeader<HTTPHeader::ContentLength> parseContentLength(const std::string& request) {
    size_t pos = request.find("\r\nContent-Length: ");
    if (pos == std::string::npos) return {};
    size_t lenStart = pos + 18;
    size_t lenEnd = request.find("\r\n", lenStart);
    std::string rawLength(request.begin() + lenStart, request.begin() + lenEnd);
    return std::stoull(rawLength);
}

using RequestBody = std::optional<std::string>;

inline RequestBody parseBody(const std::string& request) {
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart == std::string::npos) return {};
    return request.substr(bodyStart + 4);
}

struct PostRequest {
    ParameterList params;
    std::string path;
    HeaderValueType<HTTPHeader::Host> host;
    PotentialHeader<HTTPHeader::ContentType> contentType;
    PotentialHeader<HTTPHeader::ContentLength> contentLength;
    PotentialHeader<HTTPHeader::Cookie> cookies;
    RequestBody body;

    PostRequest(const std::string& request) {
        parsePath(params, path, request);
        host = parseHost(request);
        contentType = parseContentType(request);
        contentLength = parseContentLength(request);
        cookies = parseCookies(request);
        body = parseBody(request);
    }
};

struct PutRequest {
    ParameterList params;
    std::string path;
    HeaderValueType<HTTPHeader::Host> host;
    PotentialHeader<HTTPHeader::ContentType> contentType;
    PotentialHeader<HTTPHeader::ContentLength> contentLength;
    PotentialHeader<HTTPHeader::Cookie> cookies;
    RequestBody body;

    PutRequest(const std::string& request) {
        parsePath(params, path, request);
        host = parseHost(request);
        contentType = parseContentType(request);
        cookies = parseCookies(request);
        contentLength = parseContentLength(request);
        body = parseBody(request);
    }
};

struct PatchRequest {
    ParameterList params;
    std::string path;
    HeaderValueType<HTTPHeader::Host> host;
    PotentialHeader<HTTPHeader::ContentType> contentType;
    PotentialHeader<HTTPHeader::ContentLength> contentLength;
    PotentialHeader<HTTPHeader::Cookie> cookies;
    RequestBody body;

    PatchRequest(const std::string& request) {
        parsePath(params, path, request);
        host = parseHost(request);
        contentType = parseContentType(request);
        contentLength = parseContentLength(request);
        cookies = parseCookies(request);
        body = parseBody(request);
    }
};

template<HTTPMethod method>
using RequestObject = std::conditional_t<method == HTTPMethod::Get, GetRequest,
                    std::conditional_t<method == HTTPMethod::Post, PostRequest, 
                    std::conditional_t<method == HTTPMethod::Put, PutRequest,
                    std::conditional_t<method == HTTPMethod::Patch, PatchRequest, 
                    std::conditional_t<method == HTTPMethod::Delete, DeleteRequest, void>>>>>;

template<typename T>
concept RequestType = requires(T t) {
    {t.params} -> std::same_as<ParameterList>;
    {t.host} -> std::same_as<HeaderValueType<HTTPHeader::Host>>;
    {t.path} -> std::same_as<std::string>;

    requires std::is_constructible_v<T, const std::string&>;
};

enum class HTTPStatusCode {
    Ok,
    Created,
    NoContent,
    BadRequest,
    NotFound,
    MethodNotAllowed,
    InternalServerError
};

inline constexpr int statusCodeNumber(HTTPStatusCode code) {
    switch (code) {
        case HTTPStatusCode::Ok: return 200;
        case HTTPStatusCode::Created: return 201;
        case HTTPStatusCode::NoContent: return 204;
        case HTTPStatusCode::BadRequest: return 400;
        case HTTPStatusCode::NotFound: return 404;
        case HTTPStatusCode::MethodNotAllowed: return 405;
        case HTTPStatusCode::InternalServerError: return 500;
        default: return -1;
    }
}

inline constexpr std::string statusCodeString(HTTPStatusCode code) {
    switch (code) {
        case HTTPStatusCode::Ok: return "OK";
        case HTTPStatusCode::Created: return "Created";
        case HTTPStatusCode::NoContent: return "No Content";
        case HTTPStatusCode::BadRequest: return "Bad Request";
        case HTTPStatusCode::NotFound: return "Not Found";
        case HTTPStatusCode::MethodNotAllowed: return "Method Not Allowed";
        case HTTPStatusCode::InternalServerError: return "Internal Server Error";
        default: return "Invalid";
    }
}

using ResponseBody = std::optional<std::string>;

template<>
struct std::formatter<PotentialHeader<HTTPHeader::ConnectionClose>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const PotentialHeader<HTTPHeader::ConnectionClose>& p, std::format_context& ctx) const {
        if (p.has_value()) {
            return std::format_to(ctx.out(), "Connection: close\r\n");
        }
        return std::format_to(ctx.out(), "");
    }
};

template<>
struct std::formatter<PotentialHeader<HTTPHeader::ContentType>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const PotentialHeader<HTTPHeader::ContentType>& p, std::format_context& ctx) const {
        if (p.has_value()) {
            return std::format_to(ctx.out(), "Content-Type: {}\r\n", p.value());
        }
        return std::format_to(ctx.out(), "");
    }
};

template<>
struct std::formatter<std::vector<HeaderValueType<HTTPHeader::SetCookie>>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid format");
        }
        return it;
    }

    auto format(const std::vector<HeaderValueType<HTTPHeader::SetCookie>>& p, std::format_context& ctx) const {
        std::stringstream formatting;
        for (const auto& cookie : p) {
            formatting << std::format("Set-Cookie: {}={}; Max-Age={}\r\n",
                                    cookie.key, cookie.value, cookie.maxAge);
        }
        return std::format_to(ctx.out(), "{}", formatting.str());
    }
};

inline std::string getRawBody(ResponseBody body) {
    if (body.has_value()) return body.value();
    return "";
}

inline size_t getBodySize(ResponseBody body) {
    if (body.has_value()) return body.value().size();
    return 0;
}

#ifndef HOT_RELOAD
std::unordered_map<std::string, std::string> fileCache;
#endif

inline std::string loadFile(const std::string& path) {
    std::ifstream fileStream(path);
    std::stringstream buffStream;
    buffStream << fileStream.rdbuf();
    return buffStream.str();
}

struct HTTPResponse {
    HTTPStatusCode code;
    PotentialHeader<HTTPHeader::ConnectionClose> connectionClose = {};
    PotentialHeader<HTTPHeader::ContentType> contentType = {};
    std::vector<HeaderValueType<HTTPHeader::SetCookie>> cookies;
    ResponseBody body = {};

    std::string parse() {
        return std::format(
                "HTTP/1.1 {} {}\r\n"
                    "{}\r\nContent-Length: {}\r\n{}{}\r\n{}",
                    statusCodeNumber(code),
                    statusCodeString(code),
                    getHTTPDate(),
                    getBodySize(body),
                    contentType,
                    cookies,
                    getRawBody(body));
    }

    HTTPResponse(HTTPStatusCode code) : code(code) {}

    HTTPResponse& closeConnection() {
        connectionClose = {nullptr};
        return *this;
    }

    HTTPResponse& addContentType(HTTPContentType type) {
        contentType = type;
        return *this;
    }

    HTTPResponse& addBody(const std::string& responseBody) {
        body = responseBody;
        return *this;
    }

    HTTPResponse& addCookie(const ResponseCookie& cookie) {
        cookies.push_back(cookie);
        return *this;
    }

    static HTTPResponse renderHTML(const std::string& path) {
        #ifdef HOT_RELOAD
        return HTTPResponse(HTTPStatusCode::Ok)
                .addContentType(HTTPContentType::TextPlain)
                .addBody(loadFile(path))
                .closeConnection();
        #else
        if (!fileCache.contains(path)) {
            fileCache[path] = loadFile(path);
        }
        return HTTPResponse(HTTPStatusCode::Ok)
                .addContentType(HTTPContentType::TextHtml)
                .addBody(fileCache[path])
                .closeConnection();
        #endif
    }

    static HTTPResponse executeJSON(const std::string& path) {
        #ifdef HOT_RELOAD
        return HTTPResponse(HTTPStatusCode::Ok)
                .addContentType(HTTPContentType::ApplicationJson)
                .addBody(loadFile(path))
                .closeConnection();
        #else
        if (!fileCache.contains(path)) {
            fileCache[path] = loadFile(path);
        }
        return HTTPResponse(HTTPStatusCode::Ok)
                .addContentType(HTTPContentType::ApplicationJson)
                .addBody(fileCache[path])
                .closeConnection();
        #endif
    }
};

template<HTTPMethod method>
using RequestHandler = std::function<HTTPResponse(RequestObject<method>)>;

template<HTTPMethod method>
using EndpointMap = std::unordered_map<std::string, RequestHandler<method>>;

template<HTTPMethod method>
EndpointMap<method> endpointMap{};

template<HTTPMethod method>
void registerEndpoint(const std::string& path, RequestHandler<method> handler) {
    endpointMap<method>[path] = handler;
}

template<HTTPMethod method>
void handleMethodRequest(RequestObject<method> requestObject, Request& request) {
    if (endpointMap<method>.contains(requestObject.path)) {
        request.respond(endpointMap<method>[requestObject.path](requestObject).parse());
    }
    else {
        HTTPResponse response(HTTPStatusCode::NotFound);
        response.closeConnection();
        request.respond(response.parse());
    }
}

inline void handleRequest(Request& request) {
    auto content = request.readUntil("\r\n\r\n");
    auto contentLength = parseContentLength(content);
    std::string body;
    if (contentLength.has_value() && contentLength.value() > 0) {
        size_t bodyRead = content.size() - (content.find("\r\n\r\n") + 4);
        size_t bodyLeft = contentLength.value() - bodyRead;
        body = request.readRequest(bodyLeft);
    }
    content += body;
    auto method = getMethod(content);
    switch (method) {
        case HTTPMethod::Get:
            handleMethodRequest<HTTPMethod::Get>(GetRequest(content), request);
            break;
        case HTTPMethod::Post:
            handleMethodRequest<HTTPMethod::Post>(PostRequest(content), request);
            break;
        case HTTPMethod::Put:
            handleMethodRequest<HTTPMethod::Put>(PutRequest(content), request);
            break;
        case HTTPMethod::Patch:
            handleMethodRequest<HTTPMethod::Patch>(PatchRequest(content), request);
            break;
        case HTTPMethod::Delete:
            handleMethodRequest<HTTPMethod::Delete>(DeleteRequest(content), request);
            break;
        default: {
            closeServer();
            exit(1);
        }
    }
}

#endif