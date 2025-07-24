#pragma once

#include <string>
#include <vector>
#include <map>
#include <boost/locale.hpp>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <locale>
#include <codecvt>

#include "database.h"


class Indexer
{
public:
	Indexer();
	
	void save_to_database(const std::string& url, const std::string& title,
		const std::map<std::string, int>& word_f,DataBase& db);

	std::string clean_page(const std::string& html_page);
	std::map<std::string, int> count_words(const std::string& html_page);
	std::string clean_for_db(const std::string& input);

	std::string get_title(const std::string& html_page);


};
