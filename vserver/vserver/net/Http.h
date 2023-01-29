#ifndef __VS_NET_HTTP_H
#define __VS_NET_HTTP_H

namespace vs {
namespace net {

class Http {
public:
    enum class Method {
        DELETE,
        GET,
        HEAD,
        POST,
        PUT,
        INVALID
    };
    enum class Version {
        Http10, 
        Http11, 
        Unknown
    };
    enum class StatusCode {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
        UNKNOWN
    };
};

}
}

#endif