#include <iostream>
//#include <Windows.h>


#include "database.h"
#include "ini_parser.h"
#include "indexer.h"
#include "spider.h"
#include "search_program.h"
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
            //"https://metanit.com/common/",
           // "https://www.postgresql.org/docs/current/sql-insert.html",
           // "https://en.cppreference.com",
           // "https://learn.microsoft.com/ru-ru/cpp/overview/visual-cpp-in-visual-studio?view=msvc-170",
           // "https://en.cppreference.com",
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

        run_search_server(ini_parser, db);

        run_spider(ini_parser, db);

       

        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::string url = "http://localhost:" + std::to_string(ini_parser.get_port());
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



