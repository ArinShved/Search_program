#include <iostream>

#include "database.h"
#include "ini_parser.h"
#include "indexer.h"
#include "spider.h"
#include "search_program.h"
#include "search_server.h"


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
            true
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
    try
    {
        INIParser ini_parser("spider.ini");
        DataBase db(ini_parser.db_conn_str());

        run_search_server(ini_parser,db);
        run_spider(ini_parser, db);

        return 0;

    }
    catch (const std::exception& e) 
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
   
}



