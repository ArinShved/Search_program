#include "search_server.h"

using tcp = boost::asio::ip::tcp;

SearchServer::SearchServer(SearchProgram& engine, unsigned short port) : program(engine), port(port) {}

void SearchServer::run() 
{
    try 
    {
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, {tcp::v4(), port} };

        std::cout << "Search server running on port " << port << std::endl;

        while (true) 
        {
            beast::tcp_stream stream(ioc);
            acceptor.accept(stream.socket());

            try 
            {
                handle_request(stream);
            }
            catch (const std::exception& e) 
            {
                std::cerr << "Error handling request: " << e.what() << "\n";

                http::response<http::string_body> res{ http::status::internal_server_error, 11 };

                res.set(http::field::content_type, "text/plain");
                res.body() = "Internal Server Error";
                res.prepare_payload();
                http::write(stream, res);
            }
        }
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << "\n";
        throw;
    }
}

void SearchServer::handle_request(beast::tcp_stream& stream) //протестировать
{
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::read(stream, buffer, req);

    if (req.method() == http::verb::get) 
    {
        handle_get(req, stream);
    }
    else 
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::content_type, "text/plain");
        res.body() = "Invalid request method";
        res.prepare_payload();
        http::write(stream, res);
    }
}

void SearchServer::handle_get(http::request<http::string_body>& req, beast::tcp_stream& stream)
{
    if (req.target().starts_with("/search")) 
    {
        handle_search(req, stream);
    }
    else {
        http::response<http::string_body> res{ http::status::ok, req.version() };
        res.set(http::field::content_type, "text/html");
       //TODO


        http::write(stream, res);
    }
}



void SearchServer::handle_search(http::request<http::string_body>& req, beast::tcp_stream& stream)
{
    std::vector<std::string> query_words;

    auto pos = req.target().find('?');
    if (pos != std::string::npos) 
    {
        std::string str = req.target().substr(pos + 1);
        std::vector<std::string> params;
        boost::split(params, str, boost::is_any_of("&"));

        for (const auto& param : params) 
        {
            if (param.starts_with("q=")) 
            {
                std::string query = param.substr(2);
                boost::replace_all(query, "+", " ");

               
                std::istringstream iss(query);
                std::string word;
                while (iss >> word) 
                {
                    query_words.push_back(word);
                }
                break;
            }
        }
    }

 
    auto results = program.search_result(query_words);


    std::ostringstream html;

    //TODO
    
}