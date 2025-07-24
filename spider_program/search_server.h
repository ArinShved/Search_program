#pragma once


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>

//#include "search_program.h"
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

private:
    void request(beast::tcp_stream& stream);
    void get(http::request<http::string_body>& req, beast::tcp_stream& stream);
    void post(http::request<http::string_body>& req, beast::tcp_stream& stream);

    //SearchProgram& program;
    DataBase& db;
    std::mutex mutex;

    unsigned short port;
    net::io_context ioc{ 1 };
    std::string err_mes;
};