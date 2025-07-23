#include "database.h"


 DataBase::DataBase(const std::string& conn_str) : c(conn_str) 
 {
     try {
         if (!c.is_open())
         {
             throw std::runtime_error("Connection failed");
         }

         pqxx::work w(c);
         w.exec("SET client_encoding TO 'UTF-8'");
         //PQsetClientEncoding(w, "UTF8");
         w.commit();

         create_table();
     }
     catch (const pqxx::broken_connection& e) 
     {
         throw std::runtime_error("Database connection failed: " + std::string(e.what()));
     }
}

bool DataBase::database_connect(const std::string& conn_str)
{
    try
    {
        if (c.is_open()) 
        {
            c.close();
        }

        c = pqxx::connection(conn_str);
        return c.is_open();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
};


void DataBase::create_table() 
{
    try 
    {
        pqxx::work w(c);

        w.exec("CREATE TABLE IF NOT EXISTS words ("
            "id SERIAL PRIMARY KEY, "
            "word VARCHAR(64) UNIQUE NOT NULL)");

        w.exec("CREATE TABLE IF NOT EXISTS documents ("
            "id SERIAL PRIMARY KEY, "
            "url VARCHAR(2048) UNIQUE NOT NULL,"
            "title VARCHAR(512) )");

        w.exec("CREATE TABLE IF NOT EXISTS document_word ("
            "document_id INTEGER REFERENCES documents(id) ON DELETE CASCADE, "
            "word_id INTEGER REFERENCES words(id) ON DELETE CASCADE, "
            "frequency INTEGER NOT NULL, "
            "PRIMARY KEY(document_id, word_id))");

        w.commit();
    }
    catch (pqxx::sql_error e)
    {
        std::cerr << e.what() << "\n";
    }

};

void DataBase::insert_document(const std::string& url, const std::string& title)
{
     
     pqxx::work w(c);
     try
     {
        w.exec_params("INSERT INTO documents (url, title) VALUES ($1, $2) "
                      "ON CONFLICT (url) DO UPDATE SET title = EXCLUDED.title",
                      url, title);

        w.commit();
    }
    catch (const std::exception& e)
    {
        w.abort();
        throw std::runtime_error("Failed to insert document: " + std::string(e.what()) + "\n");
    }
}

void DataBase::insert_word(const std::string& word)
{
    pqxx::work w(c);
    try
    {
        
        pqxx::result r = w.exec_params("INSERT INTO words (word) VALUES ($1) RETURNING id", word);

        w.commit();
    }
    catch (const std::exception& e) 
    {
        w.abort();
        throw std::runtime_error("Failed to insert word: " + std::string(e.what()) + "/n");
    }
}

void DataBase::insert_document_word(int doc_id, int word_id, int frequency)
{
    pqxx::work w(c);
    try 
    {
        
        w.exec_params("INSERT INTO document_word (document_id, word_id, frequency) "
                      "VALUES ($1, $2, $3) "
                      "ON CONFLICT (document_id, word_id) DO UPDATE SET frequency = $3",
                      doc_id, word_id, frequency);

        w.commit();
    }
    catch (const std::exception& e)
    {
        w.abort();
        throw std::runtime_error("Failed to insert document-word relation: " + std::string(e.what()) + "/n");
    }
}


int DataBase::get_documentID(const std::string& url)
{
    try
    {
        pqxx::work w(c);
        pqxx::result r = w.exec_params("SELECT id FROM documents WHERE url = $1", url);


        if (r.empty() || r[0][0].is_null()) 
        {
            throw std::runtime_error("Document not found: " + url);
            w.abort();
          //  return -1; 
        }

        
        return r[0][0].as<int>();
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Failed to find ID: " + std::string(e.what()) + "/n");
    }
}

int DataBase::get_wordID(const std::string& word)
{
    try
    {
        pqxx::work w(c);
        pqxx::result r = w.exec_params("SELECT id FROM words WHERE word = $1", word);

        if (r.empty() ||r[0][0].is_null()) 
        {
        
            throw std::runtime_error("Word not found: " + word);
            w.abort();
           // return -1;
        }

        return r[0][0].as<int>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to find ID: " + std::string(e.what()) + "/n");
    }
}

std::vector<SearchResult> DataBase::search(const std::vector<std::string>& words)
{
    std::vector<SearchResult> results;

    try
    {
        pqxx::work w(c);

    
        std::string query = "SELECT d.url, d.title, SUM(dw.frequency) as relevance "
                            "FROM documents d "
                            "JOIN document_word dw ON d.id = dw.document_id "
                            "JOIN words w ON dw.word_id = w.id "
                            "WHERE w.word IN (";

        for (int i = 0; i < words.size(); ++i) 
        {
            if (i >0)
            {
                query += ", ";
            }
            query += "$" + std::to_string(i + 1);
        }

        query += ") GROUP BY d.url, d.title "
                 "HAVING COUNT(DISTINCT w.word) = " + std::to_string(words.size()) + " "
                 "ORDER BY relevance DESC "
                 "LIMIT 10";

        
        pqxx::params params;

        for (const auto& word : words)
        {
            params.append(word);
        }

        
        pqxx::result r = w.exec_params(query, params);

        for (const auto& row : r)
        {
            SearchResult result;

            result.url = row["url"].as<std::string>();
            result.title = row["title"].as<std::string>();
            result.relevance = row["relevance"].as<int>();
            results.push_back(result);
        }

        w.commit();
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Database search failed: " + std::string(e.what()));
    }

    return results;
}


pqxx::connection& DataBase::get_connection()
{
    return c;
}

void DataBase::clear_database() 
{
    try 
    {
        pqxx::work w(c);

        w.exec("DELETE FROM document_word");
        w.exec("DELETE FROM documents");
        w.exec("DELETE FROM words");


        w.commit();
        
        std::cout << "Database cleared\n";
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to clear database: " + std::string(e.what()) + "\n");
    }
}

DataBase::~DataBase() 
{
    if (c.is_open()) 
    {
        c.close(); 
    }
}
