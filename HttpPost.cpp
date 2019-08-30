#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <regex>
#include <stdlib.h>
#include <string>
#include "HttpPost.h"
#include "JsonWrapper.h"
#include "Loger.h"

HttpPost& HttpPost::Instance() {
    static HttpPost _instance;
    return _instance;
}

void HttpPost::PostNotice(boost::property_tree::ptree& notice) {
    if (m_stop_post) {
        return;
    }
    std::string content = JsonWrapper::ToJsonStr(notice);
    try {
        boost::shared_ptr<boost::thread> thread(
            new boost::thread(boost::bind(&HttpPost::Post, &HttpPost::Instance(), content)));
    } catch (std::exception&) {
    }
}

void HttpPost::SetUrl(const std::string& url) {
    FUNC_WARDER;

    m_synchronizer.Lock();
    m_server_info.m_url = url;
    std::regex ex("(((http)://)?)([^/ :]+):?([^/ ]*)(/?.*)");

    std::smatch what;
    if (!std::regex_match(url, what, ex)) {
        return;
    }

    m_server_info.m_scheme = what[3];
    m_server_info.m_host = what[4];
    m_server_info.m_port = what[5];
    m_server_info.m_path = what[6];

    if (m_server_info.m_scheme.empty()) {
        m_server_info.m_scheme = "http";
    }

    if (m_server_info.m_port.empty()) {
        m_server_info.m_port = "80";
    }

    if (m_server_info.m_path.empty()) {
        m_server_info.m_path = "/";
    }
    m_synchronizer.Unlock();
}

void HttpPost::Stop() {
    FUNC_WARDER;
    m_stop_post = true;
}

void HttpPost::Post(std::string& content) {
    m_synchronizer.Lock();
    std::string host = m_server_info.m_host;
    std::string port = m_server_info.m_port;
    std::string path = m_server_info.m_path;
    m_synchronizer.Unlock();

    try {
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(m_io_context);
        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(query, ec);
        if (ec) {
            return;
        }

        // Try each endpoint until we successfully establish a connection.
        boost::asio::ip::tcp::socket socket(m_io_context);
        boost::asio::connect(socket, endpoints, ec);
        if (ec) {
            return;
        }

        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n";
        request_stream << "Content-Length: " << content.length() << "\r\n";
        request_stream << "Content-Type: application/json\r\n\r\n";
        request_stream << content;
        boost::asio::write(socket, request, ec);
        if (ec) {
            socket.close();
            return;
        }

        boost::asio::streambuf response;
        static const std::string delimiter("\r\n");
        size_t n = boost::asio::read_until(socket, response, delimiter, ec);
        if (ec) {
            socket.close();
            return;
        }

        LOG("Notification response: %.*s", n - delimiter.length(), response.data());
        response.consume(n);
        socket.close();
    } catch (...) {
        LOG_LINE;
    }
}
