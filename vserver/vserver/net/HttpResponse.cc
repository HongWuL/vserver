#include "HttpResponse.h"
#include <sstream>
#include "Log.h"

namespace vs {
namespace net {

HttpResponse::HttpResponse(bool close) :
    m_version(Http::Version::Http11),
    m_closeConnection(close),
    m_statueCode(Http::StatusCode::UNKNOWN) {
}

std::string HttpResponse::dump() {
    std::stringstream ss;
    //version code state
    ss  << getVersionMessage() << " " 
        << static_cast<int>(m_statueCode) << " "
        << getStatusMessage() << "\r\n";
    
    if(m_closeConnection) {
        ss << "Connection: close\r\n";
    }
    else {
        ss << "Content-Length: " << m_body.size() << "\r\n";
        ss << "Connection: Keep-Alive\r\n";
    }
    for(const auto &header: m_headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }

    ss << "\r\n";
    ss << m_body;
    GLOG_INFO << "HttpResponse::dump()\n" << ss.str();
    
    return ss.str();
}       

}
}