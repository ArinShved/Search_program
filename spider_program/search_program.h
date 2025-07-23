#pragma once

#include <vector>
#include <string>

#include "database.h"
#include "indexer.h"


class SearchProgram {
public:
    explicit SearchProgram(DataBase& db);

    std::vector<SearchResult> search_result(const std::vector<std::string>& query_words, int limit = 10);
    void index_document(const std::string& url, const std::string& content);

private:
    DataBase& db;

    std::vector<std::string> extract_words(const std::string& text);

};
