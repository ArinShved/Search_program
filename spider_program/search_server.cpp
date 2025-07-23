#include "search_server.h"

using tcp = boost::asio::ip::tcp;

// проверить коректность закрытия

SearchServer::SearchServer(SearchProgram& engine, unsigned short port) : program(engine), port(port) {}

void SearchServer::run() 
{
    try 
    {
        tcp::acceptor acceptor{ ioc, {tcp::v4(), port} };

        std::cout << "Search server running on port " << port << std::endl;

        while (true) 
        {
            beast::tcp_stream stream(ioc);
            acceptor.accept(stream.socket());

            try 
            {
                request(stream);
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

void SearchServer::request(beast::tcp_stream& stream) 
{
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::read(stream, buffer, req);

    if (req.method() == http::verb::get) 
    {
        get(req, stream);
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

void SearchServer::get(http::request<http::string_body>& req, beast::tcp_stream& stream)
{
    if (req.target().starts_with("/search")) 
    {
        post(req, stream);
    }
    else {
        http::response<http::string_body> res{ http::status::ok, req.version() };
        res.set(http::field::content_type, "text/html");
        res.body() = R"(
<html>
<head>
    <style>
        body {
            background-color: #e6f2ff; 
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        
        h1 {
            font-size: 2.5rem; 
            color: #0066cc;
            margin-bottom: 30px;
            text-align: center;
        }
        
        form {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 80%;
            max-width: 500px;
            text-align: center;
        }
        
        input[type="text"] {
            width: 80%;
            padding: 12px;
            font-size: 1.2rem;
            border: 2px solid #99ccff;
            border-radius: 5px;
            margin-bottom: 20px;
        }
        
        button {
            background-color: #4da6ff; 
            color: white;
            border: none;
            padding: 12px 25px;
            font-size: 1.2rem;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        
        button:hover {
            background-color: #3399ff; 
        }
    </style>
</head>
<body>
    <h1>Search Server</h1>
    <form action="/search" method="get">
        <input type="text" name="q" placeholder="Enter search terms">
        <button type="submit">Search</button>
    </form>
</body>
</html>
        )";
        res.prepare_payload();


        http::write(stream, res);
    }
}



void SearchServer::post(http::request<http::string_body>& req, beast::tcp_stream& stream)
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
    html << R"(
       <html>
        <head>
            <title>Search Results</title>
            <style>
                body {
                    background-color: #e6f2ff;
                    font-family: Arial, sans-serif;
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    min-height: 100vh;
                    margin: 0;
                    padding: 20px;
                }
                .container {
                    background-color: white;
                    padding: 30px;
                    border-radius: 10px;
                    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                    width: 80%;
                    max-width: 800px;
                }
                h1 {
                    color: #0066cc;
                    text-align: center;
                    margin-bottom: 20px;
                }
                .search-form {
                    margin-bottom: 30px;
                    text-align: center;
                }
                input[type="text"] {
                    width: 70%;
                    padding: 12px;
                    font-size: 1.1rem;
                    border: 2px solid #99ccff;
                    border-radius: 5px;
                    margin-right: 10px;
                }
                button {
                    background-color: #4da6ff;
                    color: white;
                    border: none;
                    padding: 12px 25px;
                    font-size: 1.1rem;
                    border-radius: 5px;
                    cursor: pointer;
                    transition: background-color 0.3s;
                }
                button:hover {
                    background-color: #3399ff;
                }
                .result {
                    background: #f5f9ff;
                    padding: 15px;
                    margin-bottom: 15px;
                    border-radius: 5px;
                    border-left: 4px solid #4da6ff;
                }
                .title {
                    font-weight: bold;
                    font-size: 1.2rem;
                    color: #003366;
                    margin-bottom: 5px;
                }
                .url {
                    color: #008000;
                    font-size: 0.9rem;
                    margin-bottom: 5px;
                    word-break: break-all;
                }
                .relevance {
                    color: #666;
                    font-size: 0.8rem;
                }
                .no-results {
                    text-align: center;
                    color: #666;
                    font-style: italic;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>Search Results</h1>
                <div class="search-form">
                    <form action="/search" method="get">
                        <input type="text" name="q" value=")";



    if (!query_words.empty()) 
    {
        for (size_t i = 0; i < query_words.size(); ++i) 
        {
            if (i > 0) html << " ";
            html << query_words[i];
        }
    }

    html << R"(" placeholder="Enter search terms">
                        <button type="submit">Search</button>
                    </form>
                </div>
                <div class="results">)";

    if (results.empty()) 
    {
        html << "<p>No results found</p>";
    }
    else 
    {
        html << "<div class=\"results\">";
        for (const auto& result : results) 
        {
            html << R"(
                <div class="result">
                    <div class="title">)" << result.title << R"(</div>
                    <div class="url">)" << result.url << R"(</div>
                    <div class="relevance">Relevance: )" << result.relevance << R"(</div>
                </div>
            )";
        }
        html << "</div>";
    }

    html << R"(
        </body>
        </html>
    )";

    http::response<http::string_body> res{ http::status::ok, req.version() };
    res.set(http::field::content_type, "text/html");
    res.body() = html.str();
    res.prepare_payload();
    http::write(stream, res);
    
}

SearchServer::~SearchServer() 
{
    ioc.stop();
}