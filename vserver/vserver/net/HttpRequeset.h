#ifndef __VS_NET_HTTPREQUESET_H
#define __VS_NET_HTTPREQUESET_H

#define CR '\r' //回车
#define LF '\n' //换行

#include <unordered_map>
#include <iostream>
#include <sstream>
#include "Http.h"

namespace vs {
namespace net {

class HttpRequest {
public:

    enum class ParseState {
        INVALID,                //无效
        INVALID_METHOD,         //无效请求方法
        INVALID_PATH,           //无效的请求路径
        INVALID_VERSION,        //无效的协议版本号
        INVALID_HEADER,         //无效请求头
        
        START,                  //请求行开始

        METHOD,                 //请求方法
        
        BEFORE_PATH,            //PATH前的空格
        PATH,                   //请求路径
        
        BEFORE_PARAM_KEY,
        PARAM_KEY,
        BEFORE_PARAM_VALUE,
        PARAM_VALUE,

        BEFORE_VERSION,         //
        VERSION,                //协议版本

        HEADER_KEY,             //请求头key
        BEFORE_HEADER_VALUE,    //
        HEADER_VALUE,           //请求头value

        W_CR,
        W_CR_LF,
        W_CR_LF_CR,

        BODY,                   //请求体
        END,                    //解析结束

    };

    void setMethod(const char* start, const char *end) {
        std::string m(start, end);
        if (m == "DELETE") m_method = Http::Method::DELETE;
        else if (m == "GET") m_method = Http::Method::GET;
        else if (m == "HEAD") m_method = Http::Method::HEAD;
        else if (m == "POST") m_method = Http::Method::POST;
        else if (m == "PUT") m_method = Http::Method::PUT;
        else m_method = Http::Method::INVALID;
    }
    
    void setVersion(const char* start, const char *end) {
        std::string m(start, end);
        if(m == "HTTP/1.1") m_version = Http::Version::Http11;
        else if(m == "HTTP/1.0") m_version = Http::Version::Http10;
        else m_version = Http::Version::Unknown;
    }

    void setPath(const char* start, const char *end) {
        std::string m(start, end);
        m_path = m;
    }

    void addParams(std::string key, std::string val) {
        m_params.insert({key, val});
    }
    void addHeader(std::string key, std::string val) {
        m_heads.insert({key, val});
    }
    std::string info() const{
        std::stringstream ss;
        ss << "[method] " << static_cast<int>(m_method) << std::endl;
        ss << "[version] " << static_cast<int>(m_version) << std::endl;
        ss << "[path] " << m_path << std::endl;
        ss << "[params] " << std::endl;
        for(auto &x: m_params) {
            ss << x.first << ": " << x.second << std::endl; 
        }
        ss << "[headers] " << std::endl;
        for(auto &x: m_heads) {
            ss << x.first << ": " << x.second << std::endl; 
        }
        ss << "[body] " << std::endl;
        ss << m_body << std::endl;
        return ss.str();
    }

    std::string getHeader(std::string key) {
        std::string res;
        auto it = m_heads.find(key);
        if(it != m_heads.end()) {
            res = it->second;
        }
        return res;
    }
    Http::Version getVersion() const { return m_version;}
    Http::Method getMethod() const { return m_method;}
    std::string getPath() const { return m_path; };
    
    void parse(const char* msg, int len);

private:
    Http::Method m_method;
    Http::Version m_version;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_params;
    std::unordered_map<std::string, std::string> m_heads;
    std::string m_body;
    ParseState m_state = ParseState::START;

};

}

}

#endif