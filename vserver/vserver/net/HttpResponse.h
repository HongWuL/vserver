#ifndef __VS_NET_HTTPRESPONSE_H
#define __VS_NET_HTTPRESPONSE_H

#include <unordered_map>
#include "Http.h"

namespace vs {
namespace net {

class HttpResponse {
public:
    HttpResponse(bool close);

    void setVersion(Http::Version version) {m_version = version;}
    void setStatus(Http::StatusCode code) {m_statueCode = code;}
    void setBody(std::string body) {m_body = body;}
    void addHeader(std::string key, std::string value) {
        m_headers.insert({key, value});
    }
    void setCloseConnection(bool on) { 
        m_closeConnection = on; 
    }

    bool closeConnection() const { return m_closeConnection; }
    Http::Version getVersion() const { return m_version;}
    Http::StatusCode getStatus() const { return m_statueCode;}
    std::string getBody() const { return m_body;}
    std::unordered_map<std::string, std::string> getHeader() const {
        return m_headers;
    }

    std::string getStatusMessage() {
        std::string statusMessage;
        if(m_statueCode == Http::StatusCode::OK) {
            statusMessage = "Ok";
        }
        else if(m_statueCode == Http::StatusCode::BAD_REQUEST) {
            statusMessage = "Bad Request";
        }
        else if(m_statueCode == Http::StatusCode::NOT_FOUND) {
            statusMessage = "Not Found";
        }
        else if(m_statueCode == Http::StatusCode::INTERNAL_SERVER_ERROR) {
            statusMessage = "Internal Server Error";
        } 
        else {
            statusMessage = "Unknown";
        }
        return statusMessage;
    }
    std::string getVersionMessage() {
        std::string versionMessage;
        if(m_version == Http::Version::Http10) {
            versionMessage = "HTTP/1.0";
        }
        else if(m_version == Http::Version::Http11) {
            versionMessage = "HTTP/1.1";
        }
        else {
            versionMessage = "Unknown";
        }
        return versionMessage;
    }

    std::string dump();

private:
    std::unordered_map<std::string, std::string> m_headers;
    Http::Version m_version;
    std::string m_versionMessage;
    Http::StatusCode m_statueCode;
    std::string m_body;
    bool m_closeConnection;
};

}
} 


#endif