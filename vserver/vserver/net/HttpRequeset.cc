#include "HttpRequeset.h"

namespace vs {
namespace net {

void HttpRequest::parse(const char* msg, int len) {
    char* p = const_cast<char*>(msg);   //
    int pos = 0;
    char* begin = p;
    std::string param_key, param_value;
    std::string header_key, header_value;
    int bodyLength = 0;

    while(m_state != ParseState::INVALID
          && m_state != ParseState::INVALID_HEADER
          && m_state != ParseState::INVALID_METHOD
          && m_state != ParseState::INVALID_PATH
          && m_state != ParseState::INVALID_VERSION
          && m_state != ParseState::END
          && pos < len
          ) {
        char c = *p;

        switch (m_state) {

        case ParseState::START: {
            if(c == CR || c == LF || isblank(c)) {}
            else if(isupper(c)) {
                begin = p;
                m_state = ParseState::METHOD;
            }
            else {
                m_state = ParseState::INVALID;
            }
            break;
        }
        case ParseState::METHOD: {
            if(isupper(c)){}
            else if(isblank(c)) {
                setMethod(begin, p);
                begin = p;
                m_state = ParseState::BEFORE_PATH;
            }
            else {
                m_state = ParseState::INVALID_METHOD;
            }
            break;
        }
        case ParseState::BEFORE_PATH: {
            if(isblank(c)){}
            else if(c == '/') {
                begin = p;
                m_state = ParseState::PATH;
            }
            else {
                m_state = ParseState::INVALID_PATH;
            }
            break;
        }
        case ParseState::PATH: {
            if(isblank(c)) {
                setPath(begin, p);
                m_state = ParseState::BEFORE_VERSION;
            }
            else if(c == '?'){
                setPath(begin, p);
                m_state = ParseState::BEFORE_PARAM_KEY;
            }
            break;
        }
        case ParseState::BEFORE_PARAM_KEY: {
            if(isblank(c) || c==LF || c==CR) {
                //'?'后面是空格、回车、换行则是无效的URL
                m_state = ParseState::INVALID_PATH;
            }
            else {
                begin = p;
                m_state = ParseState::PARAM_KEY;
            }
            break;
        }
        case ParseState::PARAM_KEY: {
            if(c == '=') {
                // param_key = std::string(begin, p);
                param_key.assign(begin, p);
                m_state = ParseState::BEFORE_PARAM_VALUE;
            }
            else if(isblank(c)) {
                m_state = ParseState::INVALID_PATH;
            }
            break;
        }
        case ParseState::BEFORE_PARAM_VALUE: {
            if(isblank(c) || c == LF || c == CR) {
                m_state = ParseState::INVALID_PATH;
            }
            else {
                begin = p;
                m_state = ParseState::PARAM_VALUE;
            }
            break;
        }
        case ParseState::PARAM_VALUE: {
            if(c == '&') {
                param_value.assign(begin, p);
                addParams(param_key, param_value);
                m_state = ParseState::BEFORE_PARAM_KEY;
            }
            else if(isblank(c)) {
                param_value.assign(begin, p);
                addParams(param_key, param_value);
                m_state = ParseState::BEFORE_VERSION;
            }
            break;  
        }

        case ParseState::BEFORE_VERSION: {
            if(isblank(c)) {}
            else {
                begin = p;
                m_state = ParseState::VERSION;
            }
            break;
        }
        case ParseState::VERSION: {
            if(c == CR) {
                setVersion(begin, p);
                m_state = ParseState::W_CR;
            }
            break;
        }
        case ParseState::HEADER_KEY: {
            if(c == ':') {
                header_key.assign(begin, p);
                m_state = ParseState::BEFORE_HEADER_VALUE;
            }
            break;
        }
        case ParseState::BEFORE_HEADER_VALUE: {
            if(!isblank(c)) {
                begin = p;
                m_state = ParseState::HEADER_VALUE;
            }
        }
        case ParseState::HEADER_VALUE: {
            if(c == CR) {
                header_value.assign(begin, p);
                addHeader(header_key, header_value);
                m_state = ParseState::W_CR;
            }
            break;
        }
        case ParseState::BODY: {
            m_body.assign(p, bodyLength);
            m_state = ParseState::END;
            break;
        }

        case ParseState::W_CR: {
            if(c == LF) {
                m_state = ParseState::W_CR_LF;
            }
            else {
                m_state = ParseState::INVALID;
            }
            break;
        }
        case ParseState::W_CR_LF: {
            if(c == CR) {
                m_state = ParseState::W_CR_LF_CR;
            }
            else if(isblank(c)) {
                m_state = ParseState::INVALID;
            }
            else {
                //请求头开始
                begin = p;
                m_state = ParseState::HEADER_KEY;
            }
            break;
        }
        case ParseState::W_CR_LF_CR: {
            if(c == LF) {
                if(m_heads.count("Content-Length")) {
                    bodyLength = stoi(m_heads["Content-Length"]);
                    if(bodyLength > 0) {
                        m_state = ParseState::BODY;
                    }
                    else {
                        m_state = ParseState::END;
                    }
                }
                else {
                    // http1.0中Content-Length可有可无
                    if(pos < len) {
                        bodyLength = len - pos - 1;
                        m_state = ParseState::BODY;
                    }
                    else {
                        m_state = ParseState::END;
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        p ++;
        pos ++;
    }
}

}
}