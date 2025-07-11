#include "indexer.h"

Indexer::Indexer() 
{
   
};

void Indexer::save_to_database(const std::string& url, const std::string& title,
                               const std::map<std::string, int>& word_freq,
                               DataBase& db)
{
        pqxx::work w(db.get_connection());

       // auto conn = db.get_connection();
        try 
        {
            std::string clean_url = clean_for_db(url);
            if (clean_url.empty()) 
            {
                throw std::runtime_error("Empty URL after cleaning");
            }

            std::string clean_title = clean_for_db(title);
            if (clean_title.empty()) 
            {
                clean_title = "No title";
            }

            
            w.exec_params("INSERT INTO documents (url, title) VALUES ($1, $2) "
                "ON CONFLICT (url) DO UPDATE SET title = EXCLUDED.title",
                clean_url, clean_title
            );

            
            pqxx::result r = w.exec_params("SELECT id FROM documents WHERE url = $1", clean_url);
            if (r.empty()) 
            {
                throw std::runtime_error("Document not found: " + clean_url);
            }

            int doc_id;
            try 
            {
                doc_id = r[0][0].as<int>(); 
            }
            catch (const pqxx::conversion_error& e) 
            {
                throw std::runtime_error("Invalid document ID format: " + std::string(e.what()));
            }

            
            w.exec_params("DELETE FROM document_word WHERE document_id = $1", doc_id);

            
            for (const auto& [word, freq] : word_freq) 
            {
                if (word.size() >= 3 && word.size() <= 32) 
                {
                    try 
                    {
                        
                        w.exec_params("INSERT INTO words (word) VALUES ($1) "
                                      "ON CONFLICT (word) DO NOTHING",word);

                        
                        w.exec_params("INSERT INTO document_word (document_id, word_id, frequency) "
                                     "SELECT $1, id, $3 FROM words WHERE word = $2",
                                     doc_id, word, freq);
                    }
                    catch (const std::exception& e) 
                    {
                        std::cerr << "Error processing word '" << word << "': " << e.what() << std::endl;
                        continue; 
                    }
                }
            }

            w.commit();
            
  
        
        }
    catch (const std::exception& e)
    {
        std::cerr << "Database save error: " << e.what() << std::endl;
        throw;  
    }
}


std::string Indexer::clean_page(const std::string& html_page)
{
    std::string text;
    bool tag = false;
    bool space = true; 

    for (char c : html_page) 
    {
        if (c == '<') 
        {
            tag = true;
            
            if (!space && !text.empty()) 
            {
                text += ' ';
                space = true;
            }
            continue;
        }
        else if (c == '>') 
        {
            tag = false;
            continue;
        }

        if (!tag) 
        {
            if (std::isspace(c) || std::ispunct(static_cast<unsigned char>(c))) 
            {
                if (!space) 
                {
                    text += ' ';
                    space = true;
                }
            }
            else 
            {
                text += std::tolower(static_cast<unsigned char>(c));
                space = false;
            }
        }
    }

    
    size_t start = text.find_first_not_of(' ');

    if (start == std::string::npos)
    {
        return "";
    }

    size_t end = text.find_last_not_of(' ');
    std::string result = text.substr(start, end - start + 1);
    return result;
    
};


std::map<std::string, int> Indexer::count_words(const std::string& html_page)
{
    std::map<std::string, int> word_f;
    std::string html = clean_page(html_page);
    std::istringstream iss(html);

    std::string w;

    while (iss >> w)
    {
        while (!w.empty() && std::ispunct(w.front()))
        {
            w.erase(0, 1);
        }

        while (!w.empty() && std::ispunct(w.back()))
        {
            w.pop_back();
        }
       
        if (w.size() >= 3 && w.size() <= 32)
        {
            if (!w.empty())
            {
                word_f[w]++;
            }
        }
    }

    return word_f;
};


std::string Indexer::get_title(const std::string& html_page) 
{
    
    std::string title;

    int tit_start = html_page.find("<title>");

    if (tit_start != std::string::npos) 
    {
        tit_start += 7;
        int tit_end = html_page.find("</title>", tit_start);

        if (tit_end != std::string::npos)
        {
            title = html_page.substr(tit_start, tit_end - tit_start);
            
            title.erase(title.find_last_not_of(" \n\r\t") + 1);
            title.erase(0, title.find_first_not_of(" \n\r\t"));
        }
    }

    boost::replace_all(title, "&amp;", "&");
    boost::to_lower(title);
    return title;
    
}

std::string Indexer::clean_for_db(const std::string& input) 
{
    if (input.empty())
    {
        return "";
    }

    std::string output;
    std::vector<std::string> arr = { "&lt;", "&gt;", "&amp;", "&quot;", "&apos;", "&nbsp;" };
    output.reserve(input.size());

    for (int i = 0; i < input.size(); ++i)
    {
        char c = input[i]; 

        if (c == '&') 
        {
           
            if (i + 4 < input.size() && input.substr(i, 5) == "&amp;")// постарайся оптимизировать
            {
                output += '&';
                i += 4;
                continue;
            }
            if (i + 3 < input.size() && input.substr(i, 4) == "&lt;")
            {
                output += '<';
                i += 3;
                continue;
            }
            if (i + 3 < input.size() && input.substr(i, 4) == "&gt;")
            {
                output += '>';
                i += 3;
                continue;
            }
            if (i + 5 < input.size() && input.substr(i, 6) == "&quot;") {
                output += '"';
                i += 5;
                continue;
            }
            if (i + 5 < input.size() && input.substr(i, 6) == "&apos;") {
                output += '\'';
                i += 5;
                continue;
            }
            if (i + 5 < input.size() && input.substr(i, 6) == "&nbsp;") {
                output += ' ';
                i += 5;
                continue;
            }
        }

       

        if (c >= 32 && c <= 126)
        { 
            output += c;
        }
    }
    return output;
}