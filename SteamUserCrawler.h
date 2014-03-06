#ifndef STEAMUSERCRAWLER_H
#define STEAMUSERCRAWLER_H

#include <stdio.h>
#include <string.h>

// for time logging
#include <sys/time.h>
#include <unistd.h>

#include <gumbo.h>

#include <iostream>
#include <string>
#include <sstream>
#include <queue>

using namespace std;

// boost
#include <boost/algorithm/string.hpp>
#include <boost/unordered_set.hpp>

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
        boost::unordered_set<string> userURLRef;
        
        struct timeval start, end;
        long calTime(struct timeval start, struct timeval end);
};

#endif /* STEAMUSERCRAWLER_H */
