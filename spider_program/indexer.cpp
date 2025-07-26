#include "indexer.h"


Indexer::Indexer() 
{
   
};

void Indexer::save_to_database(const std::string& url, const std::string& title,
                               const std::map<std::string, int>& word_freq,
                               DataBase& db)
{
       
        pqxx::work w(db.get_connection());

        try 
        {
            std::string clean_url = url;//clean_for_db(url);
            if (clean_url.empty()) 
            {
                throw std::runtime_error("Empty URL after cleaning");
            }

            std::string clean_title = clean_for_db(title);
            if (clean_title.empty()) 
            {
                clean_title = "No title";
            }

            db.insert_document(w, clean_url, clean_title);

            int doc_id = db.get_documentID(w, clean_url);
            
            db.clear_document_words(w, doc_id);
            
            for (const auto& [word, freq] : word_freq) 
            {
                if (word.size() >= 3 && word.size() <= 32) 
                {
                    try 
                    {
                      
                        db.insert_word(w, word);

                        int word_id = db.get_wordID(w, word);

                        db.insert_document_word(w, doc_id, word_id, freq);
                        
                     
                    }
                    catch (const std::exception& e) 
                    {
                        std::cerr << "Error processing word '" << word << "': " << e.what() << "\n";
                        continue; 
                    }
                }
            }

            w.commit();
            
  
        
        }
    catch (const std::exception& e)
    {
        w.abort();
        std::cerr << "Database save error: " << e.what() << "\n";
        
        throw;  
    }
}


std::string Indexer::clean_page(const std::string& html_page)
{
    std::wstring winput = utf8_to_wstring(html_page);
    std::wstring text;
    bool tag = false;
    bool space = true; 
    std::string clean_html = clean_for_db(html_page);

    for (wchar_t c : winput) 
    {

        if (c == L'<') 
        {
            tag = true;
            
            if (!space && !text.empty()) 
            {
                text += L' ';
                space = true;
            }
            continue;
        }
        else if (c == L'>') 
        {
            tag = false;
            continue;
        }

        if (!tag) 
        {
            if (iswspace(c) || iswpunct(c))
            {
                if (!space) 
                {
                    text += L' ';
                    space = true;
                }
            }
            else 
            {
                text += tolower(c);
                space = false;
            }
        }
    }

    
    size_t start = text.find_first_not_of(L' ');

    if (start == std::wstring::npos)
    {
        return "";
    }

    size_t end = text.find_last_not_of(L' ');
    std::wstring result = text.substr(start, end - start + 1);
    
    
    return wstring_to_utf8(result);
    
};


std::map<std::string, int> Indexer::count_words(const std::string& html_page)
{
    std::map<std::string, int> word_f;
    std::string html = clean_page(html_page);
    std::wstring wstr = utf8_to_wstring(html);
    

    std::wstring w;

    for (wchar_t c : wstr)
    {
        if (iswalnum(c))
        {
            w += c;
        }
        else if (!w.empty())
        {
            if (w.size() >= 3 && w.size() <= 32)
            {
                if (!w.empty())
                {
                    word_f[wstring_to_utf8(w)]++;
                }
                w.clear();
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

    std::string clean_title = clean_for_db(title);
    boost::to_lower(clean_title);
    return clean_title;
    
}


std::string Indexer::clean_for_db(const std::string& input) 
{
    if (input.empty())
    {
        return "";
    }

    std::wstring winput = utf8_to_wstring(input);

    std::wstring output;
    static const std::unordered_map<std::wstring, wchar_t> del_s = {
        {L"&amp;", L'&'},{L"&lt;", L'<'},{L"&gt;", L'>'},{L"&quot;", L'"'},
        {L"&apos;", L'\''},{L"&nbsp;", L' '},   
    };

    
    for (int i = 0; i < winput.size(); ++i)
    {
        wchar_t c = input[i]; 
        bool stop = false;

        if (c == L'&')
        {
            for (auto& [word, swap] : del_s)
            {
                if (i + word.size() <= input.size() && winput.compare(i, word.size(), word) == 0)
                {
                    output += swap;
                    i += word.size();
                    stop = true;
                    break;
                }
            }

            if (stop)
            {
                continue;
            }
        }
            
        if (iswalnum(winput[i]) || winput[i] == L' ') {
            output += towlower(winput[i]);
        }

        
    }
    return wstring_to_utf8(output);
}

std::wstring Indexer::utf8_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
    return c.from_bytes(str);
}


std::string Indexer::wstring_to_utf8(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
    return c.to_bytes(wstr);
}