
#include "ini_parser.h"

INIParser::INIParser(const std::string& _filename) : filename(_filename)
{
    db_data = parse_database();
    sp_data = parse_spider();
    server_port = parse_port();
};

std::map<std::string, std::map<std::string, std::string>> INIParser::read_INIfile()
{
    std::ifstream f(filename);

    if (!f)
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::map<std::string, std::map<std::string, std::string>> res;

    std::string current_section;

    std::string line;

    auto t = [](std::string& s)
    {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
    };

    while (std::getline(f, line)) 
    {    
        int com_pos = line.find_first_of(";#");

        if (com_pos != std::string::npos)
        {
            line.erase(com_pos);
        }

        t(line);

        if (line.empty())
        {
            continue;
        }

        if (line.front() == '[' && line.back() == ']')
        {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        
        int eq_pos = line.find('=');

        if (eq_pos == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, eq_pos);

        std::string value = line.substr(eq_pos + 1);

        t(key);
        t(value);

        res[current_section][key] = value;
    }

    return res;
}

DatabaseData INIParser::parse_database() 
{
    auto text = read_INIfile();

    DatabaseData db;

    try 
    {
        db.host = text["database"]["host"];
        db.port = std::stoi(text["database"]["port"]);
        db.dbname = text["database"]["dbname"];
        db.user = text["database"]["user"];
        db.password = text["database"]["password"];
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Invalid database text: " + std::string(e.what()) + "\n");
    }

    return db;
}

SpiderData INIParser::parse_spider()
{
    auto text = read_INIfile();

    SpiderData spider;

    try 
    {
        spider.start_url = text["spider"]["start_url"];
        spider.max_depth = std::stoi(text["spider"]["max_depth"]);
        spider.max_threads = std::stoul(text["spider"]["max_threads"]);
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Invalid spider config: " + std::string(e.what()) + "\n");
    }

    return spider;
}

int INIParser::parse_port() {
    auto text = read_INIfile();

    try 
    {
        return std::stoi(text["server"]["port"]);
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Invalid server port: " + std::string(e.what()) + "/n");
    }
}

std::string INIParser::db_conn_str()
{
    return "host=" + db_data.host +
           " port=" + std::to_string(db_data.port) +
           " dbname=" + db_data.dbname +
           " user=" + db_data.user +
           " password="+ db_data.password;

}

DatabaseData& INIParser::get_db_data() 
{ 
    return db_data; 

}

SpiderData& INIParser::get_spider_data() 
{ 
    return sp_data; 
}

int INIParser::get_port() 
{
    return server_port; 
}