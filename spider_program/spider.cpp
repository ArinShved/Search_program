#include "spider.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

Spider::Spider(const std::string& _start_url,
    int _max_depth,
    size_t _max_threads,
    size_t _max_pages,
    DataBase& _db,
    bool _domain_filter)
    : start_url(_start_url),
    max_depth(_max_depth),
    max_threads(_max_threads),
    max_pages(_max_pages),
    db(_db),
    domain_filter(_domain_filter),

    thread_pool(_max_threads) {
}


void Spider::run()
{
    add_task(start_url, 0);

    while (processed_pages < max_pages && !safe_queue.queue_empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Processed pages: " << processed_pages << "\n";
}


void Spider::add_task(const std::string& url, int depth)
{
    std::unique_lock<std::mutex> lock(queue_mutex);

    if (url.find("http://") != 0 && url.find("https://") != 0)
    {
        return;
    }

    if (visited_urls.count(url) == 0 && depth <= max_depth)
    {
        safe_queue.queue_push(std::make_pair(url, depth));
        visited_urls.insert(url);
        lock.unlock();
        thread_pool.submit([this] { process_next_data(); });
    }
}


void Spider::process_next_data()//исправить
{

   // auto task = safe_queue.queue_pop();

    std::pair<std::string, int> task;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (safe_queue.queue_empty()) 
        {
            return;
        }
        task = safe_queue.queue_pop();
    }

   
    thread_pool.submit([this, task]() 
        {
        try 
        {
            std::string html = download_page(task.first);
            if (html.empty())
            {
                throw std::runtime_error("Downloaded page empty");
            }
            else if (html.size() < 100)
            {
                throw std::runtime_error("HTML too small");
            }

            std::string clean_text = indexer.clean_page(html);
            auto word_freq = indexer.count_words(clean_text);

            std::string title = indexer.get_title(html);
            title = indexer.clean_for_db(title);

            try
            {
                indexer.save_to_database(task.first, title, word_freq, db);
            }
            catch (const std::exception& db_e)
            {
                std::cerr << "Database error processing " << task.first << ": " << db_e.what() << "\n";
                return;
            }

            auto links = extract_links(html, task.first);
            for (const auto& link : links)
            {
                add_task(link, task.second + 1);
            }

            ++processed_pages;
            std::cout << "Processed: " << task.first << " (depth: " << task.second << ")\n";

        }
        catch (const std::exception& e)
        {
            std::cerr << "Error processing " << task.first << ": " << e.what() << "\n";
        }

        });
}


std::string Spider::download_page(const std::string& url)
{
    if (url.empty()) 
    {
        throw std::runtime_error("URL is empty");
    }

    int protocol_end = url.find("://");
    if (protocol_end == std::string::npos) 
    {
        throw std::runtime_error("Invalid URL");
    }

    const std::string protocol = url.substr(0, protocol_end);
    const std::string host_and_path = url.substr(protocol_end + 3);
    const size_t path_start = host_and_path.find('/');

   
    const std::string host = (path_start == std::string::npos)
        ? host_and_path
        : host_and_path.substr(0, path_start);


    const std::string path = (path_start == std::string::npos)
        ? "/"
        : host_and_path.substr(path_start);


    const std::string port = (protocol == "https") ? "443" : "80";


    net::io_context io_context;

    try 
    {
        if (protocol == "https") 
        {
            net::ssl::context ssl_context(net::ssl::context::tlsv12_client);
            ssl_context.set_default_verify_paths();

            beast::ssl_stream<beast::tcp_stream> stream(io_context, ssl_context);

            beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

            if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) 
            {
                throw std::runtime_error("Failed to set SSL host name");
            }

            tcp::resolver resolver(io_context);
            auto const results = resolver.resolve(host, port);

            beast::get_lowest_layer(stream).connect(results);

            stream.handshake(net::ssl::stream_base::client);

            http::request<http::string_body> request{
                http::verb::get,
                path,
                11  
            };
            request.set(http::field::host, host);
            request.set(http::field::user_agent, "SpiderBot/1.0");

            http::write(stream, request);

            beast::flat_buffer buffer;
            http::response<http::string_body> response;
            http::read(stream, buffer, response);

            beast::error_code ec;
            stream.shutdown(ec);

            if (ec && ec != net::ssl::error::stream_truncated) 
            {
                throw beast::system_error{ ec };
            }

            return response.body();
        }
        else 
        {
            beast::tcp_stream stream(io_context);
            stream.expires_after(std::chrono::seconds(30));

            tcp::resolver resolver(io_context);
            auto const results = resolver.resolve(host, port);

            stream.connect(results);

            http::request<http::string_body> request{
                http::verb::get,
                path,
                11
            };
            request.set(http::field::host, host);
            request.set(http::field::user_agent, "SpiderBot/1.0");

            http::write(stream, request);

            beast::flat_buffer buf;
            http::response<http::string_body> response;
            http::read(stream, buf, response);

            beast::error_code ec;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            if (ec && ec != beast::errc::not_connected) 
            {
                throw beast::system_error{ ec };
            }

            return response.body();
        }
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Failed to download page from " + url + ": " + e.what());
    }
}


std::vector<std::string> Spider::extract_links(const std::string& html, const std::string& base_url)
{
    std::vector<std::string> links;
    std::regex link_re(R"(<a\s+href=["']\s*([^"']*?)\s*["'])", std::regex_constants::icase);

    auto begin = std::sregex_iterator(html.begin(), html.end(), link_re);
    auto end = std::sregex_iterator();

    for (std::sregex_iterator i = begin; i != end; ++i) 
    {
        std::smatch match = *i;
        std::string link = match[1].str();

      
        link.erase(link.find_last_not_of(" \t\n\r\f\v") + 1);
        link.erase(0, link.find_first_not_of(" \t\n\r\f\v"));

        if (link.empty() || link[0] == '#') 
        {
            continue;
        }

        if (link.find("://") == std::string::npos) 
        {
            if (link[0] == '/') {
                int protocol_end = base_url.find("://");
                std::string domain = base_url.substr(0, base_url.find('/', protocol_end + 3));
                link = domain + link;
            }
            else 
            {
                continue;
            }
        }

        if (domain_filter && link.find(start_url) == std::string::npos) 
        {
            continue;
        }

        links.push_back(link);
    }

    return links;
  
}

 std::unordered_set<std::string>& Spider::get_visited_urls()  
 {
    return visited_urls;
}
