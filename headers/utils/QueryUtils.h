#ifndef QUERYUTILS_H
#define QUERYUTILS_H

#include <unistd.h>
#include <locale>

struct Query {
    bool isSearchInServer;
    bool isSearchInAllServers;
    bool isGetAllInServer;
    bool isGetAllInAllServers;
    std::string search;
};

class QueryUtils {
public:
    Query* getQuery(const std::string& query) const {
        Query* q = new Query();
        if (query.substr(0, 4) != "GET " && query.substr(0, 7) != "GETALL ") {
            return nullptr;
        }

        q->isSearchInServer = false;
        q->isSearchInAllServers = false;
        q->isGetAllInServer = false;
        q->isGetAllInAllServers = false;
       
        if (query.size() >= 4) {
            q->search = query.substr(4);
        }
        if (query.size() >= 7) {
            q->isSearchInAllServers = query.substr(0, 7) == "GETALL ";
            if(q->isSearchInAllServers){
                q->search = query.substr(7);
            }
        }

        q->search.erase(0, q->search.find_first_not_of(" \t\n\r"));
        q->search.erase(q->search.find_last_not_of(" \t\n\r") + 1);

        if(q->search.empty()) {
            return nullptr;
        }

        if(q->search == "*") {
            q->isGetAllInServer = true;  
        }

        if (q->search == "**") {
            q->isGetAllInAllServers = true;
        }

        if(!q->isSearchInAllServers && !q->isGetAllInAllServers && !q->isGetAllInServer){
            q->isSearchInServer = true;
        }

        return q;
    }

    std::string availableFormats(){
        return "Available formats: GET <name> or GETALL <name> or GET * or GET **";
    }
};

#endif