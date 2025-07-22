#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <openssl/ssl.h>

#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <atomic>

#include "safe_queue.h"
#include "thread_pool.h"
#include "database.h"
#include "indexer.h"
#include "ini_parser.h"

class Spider
{
public:
    Spider(const std::string& _start_url,
        int max_depth,
        size_t max_threads,
        size_t max_pages,
        DataBase& db,
        bool domain_filter);

    ~Spider();

	void run();

private:

    void process_next_data();
    bool skip_link(const std::string& link);

    void add_task(const std::string& url, int depth);

    std::string download_page(const std::string& url);
    //void save_to_db(const std::string& url, const Indexer& result);

    std::vector<std::string> extract_links(const std::string& html, const std::string& base_url);

    std::unordered_set<std::string>& get_visited_urls();




    SafeQueue<std::pair<std::string, int>> safe_queue;
    DataBase& db;
    Indexer indexer;
    ThreadPool thread_pool;
    std::unordered_set<std::string> visited_urls;


    std::string start_url;
    int max_depth;

    
    size_t max_pages;
    std::string db_conn;

    bool domain_filter;
    size_t max_threads;

  
    std::mutex queue_mutex;

    std::mutex visited_mutex;
    std::atomic<int> processed_pages{ 0 };

   
   


};
