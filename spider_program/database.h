#pragma once

#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <queue>
#include <mutex>

struct SearchResult {
	std::string url;
	std::string title;
	int relevance = 0;
};

class DataBase 
{
public:
	explicit DataBase(const std::string& conn_str);

	void insert_document(pqxx::work& w, const std::string& url, const std::string& title);
	void insert_word(pqxx::work& w, const std::string& word);
	void insert_document_word(pqxx::work& w, int doc_id, int word_id, int frequency);
	void clear_document_words(pqxx::work& w, int doc_id);

	int get_documentID(pqxx::work& w, const std::string& url);
	int get_wordID(pqxx::work& w, const std::string& word);

	std::vector<SearchResult> search(pqxx::work& w, const std::vector<std::string>& words);
	void clear_database();


	pqxx::connection& get_connection();

	~DataBase();
private:
	std::unique_ptr<pqxx::connection> l;
	pqxx::connection c;

	bool database_reconnect(const std::string& conn_str);
	void create_table();

	std::string conn_str;
	std::mutex mutex;
	std::queue<std::shared_ptr<pqxx::connection>> connections;

};