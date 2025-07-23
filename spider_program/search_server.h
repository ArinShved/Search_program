#pragma once


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>

#include "search_program.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class SearchServer {
public:
    SearchServer(SearchProgram& engine, unsigned short port);
    void run();

private:
    void request(beast::tcp_stream& stream);
    void get(http::request<http::string_body>& req, beast::tcp_stream& stream);
    void post(http::request<http::string_body>& req, beast::tcp_stream& stream);

    SearchProgram& program;
    unsigned short port;
};