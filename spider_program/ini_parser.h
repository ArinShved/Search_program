#pragma once

#include <string>
#include <map>
#include <fstream>
#include <exception>


struct DatabaseData
{
    std::string host;
    int port;
    std::string dbname;
    std::string user;
    std::string password;
};

struct SpiderData 
{
    std::string start_url;
    int max_depth;
    size_t max_threads;
};

class INIParser
{
public:
    INIParser(const std::string& _filename);
    std::map<std::string, std::map<std::string, std::string>> read_INIfile();
    

    DatabaseData& get_db_data();
    SpiderData& get_spider_data();
    int get_port();
    std::string db_conn_str();
private:
     std::string filename;
     DatabaseData db_data;
     SpiderData sp_data;
     int server_port;

     DatabaseData parse_database();
     SpiderData parse_spider();
     int parse_port();
};