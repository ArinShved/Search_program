#pragma once


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale/encoding.hpp>
#include <exception>

#include "database.h"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class SearchServer {
public:
    SearchServer(DataBase& db, unsigned short port);
    ~SearchServer();
    void run();
    std::vector<SearchResult> search_result(const std::vector<std::string>& query_words, int limit);
    std::string decode_url(const std::string& url);

private:
    void request(beast::tcp_stream& stream);
    void get(http::request<http::string_body>& req, beast::tcp_stream& stream);
    void post(http::request<http::string_body>& req, beast::tcp_stream& stream);

    DataBase& db;
    std::mutex mutex;
    unsigned short port;
    net::io_context ioc{ 1 };
    std::string err_mes;
    bool stop = false;
};