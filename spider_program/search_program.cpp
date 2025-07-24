#include "search_program.h"

SearchProgram::SearchProgram(DataBase& db) : db(db) {}

std::vector<SearchResult> SearchProgram::search_result(const std::vector<std::string>& query_words, int limit) 
{
    if (query_words.empty()) 
    {
        return {};
    }
    std::lock_guard<std::mutex> lock(mutex);//Search error: Database search failed: Started new transaction while transaction was still active./n
    
    //добавить кодировка
    try 
    {

        return db.search(query_words);
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Search error: " << e.what() << "/n";
        return {};
    }
}

void SearchProgram::index_document(const std::string& url, const std::string& content) 
{
    try 
    {
         db.insert_document(url, "");

        int doc_id = db.get_documentID(url);

        if (doc_id == -1)
        {
           throw std::runtime_error("Failed to get document ID after insertion");
        }

       

        Indexer ind;

        std::string clean_text = ind.clean_page(content);

        auto word_freq = ind.count_words(clean_text);

        std::string title = ind.get_title(content);
        title= ind.clean_for_db(title);

        ind.save_to_database(url, title, word_freq,db);
    }
    catch (const std::exception& e) 
    {
        throw std::runtime_error("Document indexing failed: " + std::string(e.what()));
    }
}

std::vector<std::string> SearchProgram::extract_words(const std::string& text) 
{
    Indexer indexer;

    std::string clean_text = indexer.clean_page(text);
    auto word_f = indexer.count_words(clean_text);

    std::vector<std::string> words;
    for (const auto& pair : word_f) 
    {
        words.push_back(pair.first);
    }

    return words;
}
