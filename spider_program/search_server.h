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
    void handle_request(beast::tcp_stream& stream);
    void handle_get(http::request<http::string_body>& req, beast::tcp_stream& stream);
    void handle_post(http::request<http::string_body>& req, beast::tcp_stream& stream);

    SearchProgram& program;
    unsigned short port;
};