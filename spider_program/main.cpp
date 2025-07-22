#include <iostream>

#include "database.h"
#include "ini_parser.h"
#include "indexer.h"
#include "spider.h"
#include "search_program.h"
#include "search_server.h"

void open_browser(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#else 
    std::string command = "xdg-open " + url;
#endif

    std::system(command.c_str());
}


void run_spider(INIParser& ini, DataBase& db) 
{
    try 
    {
        Spider spider(
            ini.get_spider_data().start_url,
            3,
            ini.get_spider_data().max_threads,
            1000,
            db,
            false
        );

     
        spider.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Spider error: " << e.what() << "\n";
    }
}

void run_search_server(INIParser& ini, DataBase& db) 
{
    try 
    {
        SearchProgram search_program(db);
        SearchServer server(search_program, ini.get_port());
        server.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << "/n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        INIParser ini_parser("spider.ini");
        DataBase db(ini_parser.db_conn_str());

        
        run_spider(ini_parser, db);

        SearchProgram searcher(db);

        
        std::vector<std::string> query = { "evil" };
        int result_limit = 5;

        auto results = searcher.search_result(query, result_limit);

        std::cout << "Search results:" << std::endl;
        for (const auto& res : results) {
            std::cout << " - " << res.url << " (relevance: " << res.relevance << ")" << std::endl;
        }

        run_search_server(ini_parser, db);
        std::string url = "http://localhost:" + std::to_string(ini_parser.get_port());

        std::cout << "Открываю браузер: " << url << std::endl;
        open_browser(url);


        //завершилась с кодом 3221225786!
        return 0;

    }
    catch (const std::exception& e) 
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
   
}



