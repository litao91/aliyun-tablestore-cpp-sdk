#include "types.hpp"
#include "tablestore/util/mempiece.hpp"
#include "tablestore/util/logger.hpp"
#include "tablestore/util/try.hpp"

using namespace std;
using namespace aliyun::tablestore::util;

namespace aliyun {
namespace tablestore {
namespace core {
namespace http {

Endpoint::Endpoint()
  : mProtocol(HTTP)
{}


namespace {

std::optional<string> parseProtocol(Endpoint::Protocol& proto, const uint8_t*& b, const uint8_t* e)
{
    const uint8_t* c = b;
    for(; c < e && *c != ':'; ++c) {
    }
    MemPiece p(b, c - b);
    if (p == MemPiece::from("http")) {
        proto = Endpoint::HTTP;
    } else if (p == MemPiece::from("https")) {
        proto = Endpoint::HTTPS;
    } else {
        return std::optional<string>(string("unsupported protocol: ") + pp::prettyPrint(p) + string("."));
    }
    if (c < e && *c == ':') {
        ++c;
    } else {
        return std::optional<string>(string("invalid syntax of endpoint."));
    }
    for (int64_t i = 0; i < 2; ++i) {
        if (c < e && *c == '/') {
            ++c;
        } else {
            return std::optional<string>(string("invalid syntax of endpoint."));
        }
    }
    b = c;
    return std::optional<string>();
}

std::optional<string> parseHost(string& out, const uint8_t*& b, const uint8_t* e)
{
    const uint8_t* c = b;
    for(; c < e && *c != ':' && *c != '/'; ++c) {
    }
    MemPiece p(b, c - b);
    if (p.length() == 0) {
        return std::optional<string>(string("invalid syntax of endpoint."));
    }
    out = p.to<string>();
    b = c;
    return std::optional<string>();
}

std::optional<string> parsePort(string& out, const uint8_t*& b, const uint8_t* e)
{
    if (b == e) {
        return std::optional<string>();
    }
    if (*b != ':') {
        return std::optional<string>();
    }
    ++b;
    const uint8_t* c = b;
    for(; c < e && (*c >= '0' && *c <= '9'); ++c) {
    }
    MemPiece p(b, c - b);
    if (p.length() == 0) {
        return std::optional<string>(string("invalid syntax of endpoint."));
    }
    out = p.to<string>();
    b = c;
    return std::optional<string>();
}

std::optional<string> validateRootPath(const uint8_t* b, const uint8_t* e)
{
    for(; b < e && *b == '/'; ++b) {
    }
    if (b < e) {
        return std::optional<string>(string("invalid syntax of endpoint."));
    }
    return std::optional<string>();
}

} // namespace

std::optional<string> Endpoint::parse(Endpoint& out, const string& url)
{
    if (url.empty()) {
        return std::optional<string>(string("Endpoint must be nonempty."));
    }
    const uint8_t* b = (uint8_t*) url.data();
    const uint8_t* e = b + url.size();
    TRY(parseProtocol(out.mProtocol, b, e));
    TRY(parseHost(out.mHost, b, e));

    string port;
    TRY(parsePort(port, b, e));
    if (!port.empty()) {
        out.mPort = port;
    } else {
        switch(out.mProtocol) {
        case Endpoint::HTTP:
            out.mPort = "80";
            break;
        case Endpoint::HTTPS:
            out.mPort = "443";
            break;
        }
    }

    TRY(validateRootPath(b, e));
    return std::optional<string>();
}

void Endpoint::prettyPrint(string& out) const
{
    switch(mProtocol) {
    case HTTP: {
        out.append("http://");
        out.append(mHost);
        if (MemPiece::from(mPort) != MemPiece::from("80")) {
            out.push_back(':');
            out.append(mPort);
        }
        break;
    }
    case HTTPS: {
        out.append("https://");
        out.append(mHost);
        if (MemPiece::from(mPort) != MemPiece::from("443")) {
            out.push_back(':');
            out.append(mPort);
        }
        break;
    }
    }
}

} // namespace http
} // namespace core
} // namespace tablestore
} // namespace aliyun

