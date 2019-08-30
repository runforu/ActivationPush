#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <regex>
#include <string>
#include <vector>
#include "Synchronizer.h"

class ServerInfo {
public:
    std::string m_url;
    std::string m_scheme;
    std::string m_host;
    std::string m_port;
    std::string m_path;
};

class HttpPost {
public:
    static HttpPost& Instance();

    void PostNotice(boost::property_tree::ptree& notice);

    void SetUrl(const std::string& url);

    void Stop();

private:
    HttpPost() : m_stop_post(false){};

    HttpPost(const HttpPost& post){};

    void operator=(const HttpPost& post){};

    ~HttpPost(){};

    void Post(std::string& content);

private:
    boost::asio::io_context m_io_context;
    Synchronizer m_synchronizer;
    ServerInfo m_server_info;
    bool m_stop_post;
};

#endif  // !_HTTPCLIENT_H_
