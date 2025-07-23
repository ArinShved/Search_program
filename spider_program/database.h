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

	void insert_document(const std::string& url, const std::string& title);
	void insert_word(const std::string& word);
	void insert_document_word(int doc_id, int word_id, int frequency);

	int get_documentID(const std::string& url);
	int get_wordID(const std::string& word);

	std::vector<SearchResult> search(const std::vector<std::string>& words);
	void clear_database();


	pqxx::connection& get_connection();

	~DataBase();
private:
	std::unique_ptr<pqxx::connection> l;
	pqxx::connection c;

	bool database_connect(const std::string& conn_str);
	void create_table();

	std::string conn_str;
	std::mutex mutex;
	std::queue<std::shared_ptr<pqxx::connection>> connections;

};