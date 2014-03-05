#ifndef STEAMUSERCRAWLER_H
#define STEAMUSERCRAWLER_H

#include <stdio.h>
#include <string.h>

#include <gumbo.h>

#include <iostream>
#include <string>
#include <sstream>
#include <queue>

using namespace std;

// boost
#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;

#include "MySQLConnector.h"
#include "CurlConnector.h"

class SteamUserCrawler {
    public :
        SteamUserCrawler() {
            dbConn = new MySQLConnector();
            curl = new CurlConnector();
            
            userURL.push("http://steamcommunity.com/profiles/76561198047494048");
        };
        SteamUserCrawler(string seedURL, string ip, int port, string id, string pwd, string db) {
            dbConn = new MySQLConnector(ip, port, id, pwd, db);
            curl = new CurlConnector();
            
            userURL.push(seedURL);
        };
        
        bool run();
    private :
        static const unsigned int MAX_QUEUE_SIZE = 50;
    
        MySQLConnector *dbConn;
        CurlConnector *curl;
        
        queue<string> userURL;
        
        GumboVector* GetGumboChildren(GumboNode *node, GumboTag tag);
        GumboVector* GetGumboChildren(GumboNode *node, GumboTag tag, const char* attribute, const char* value);
};

#endif /* STEAMUSERCRAWLER_H */
