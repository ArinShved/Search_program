#include <iostream>
#include <csignal>
#include <atomic>
#include <future>

#include "database.h"
#include "ini_parser.h"
#include "indexer.h"
#include "spider.h"
#include "search_server.h"

//#pragma comment(lib,"shell32")



void open_browser(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#else 
    std::string command = "xdg-open " + url;
#endif

    std::system(command.c_str());

  //  ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}



void run_spider(INIParser& ini, DataBase& db) 
{
    try 
    {
        Spider spider(
            ini.get_spider_data().start_url,
            ini.get_spider_data().max_depth,
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
        SearchServer server(db, ini.get_port());
        server.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << "/n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    INIParser ini_parser("spider.ini");
    DataBase db(ini_parser.db_conn_str());
    DataBase db_search(ini_parser.db_conn_str());


    try
    {
   
        std::cout << "Please, wait...\n";

        INIParser ini_parser("spider.ini");
        DataBase db(ini_parser.db_conn_str());
        DataBase db_search(ini_parser.db_conn_str());

        Spider spider(
            ini_parser.get_spider_data().start_url,
            ini_parser.get_spider_data().max_depth,
            ini_parser.get_spider_data().max_threads,
            1000,
            db,
            false
        );

       
     
        std::thread spider_pr([&spider]() {spider.run(); });
        std::this_thread::sleep_for(std::chrono::seconds(2));

        SearchServer server(db_search, ini_parser.get_port());
        std::thread search_pr([&server]() {server.run();});
               
        std::string url = "http://localhost:" + std::to_string(ini_parser.get_port());
        open_browser(url);

        std::cout << "\n\nIf Browser doesn't open, please open the link: " + url + "\n\n";

        std::cout << "Press Enter to exit...\n";
        std::cin.ignore();

        spider.stop_spider();
        std::cout << "Spider stoped... Wait...\n";
         
        server.stop_server();
        std::cout << "Server stoped... Wait...\n";

        spider_pr.join();
        std::cout << "Spided closed\n";
        search_pr.join();
        std::cout << "All closed";


        
       
        return 0;

    }
    catch (const std::exception& e) 
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown fatal error\n ";
        return 1;
    }
   
    return 0;
}



