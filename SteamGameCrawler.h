#ifndef STEAMGAMECRAWLER_H
#define STEAMGAMECRAWLER_H

#include <stdio.h>
#include <string.h>

// for time logging
#include <sys/time.h>
#include <unistd.h>

#include <gumbo.h>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

// boost
#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;

// rapidjson
#include "rapidjson/document.h"

#include "MySQLConnector.h"
#include "CurlConnector.h"

static const int cc_size = 26;
static const char * const cc[] = {
	"us", 
	"uk",
	"au",
	"jp", 
	"kr",
	"bg", 
	"cz", 
	"dk", 
	"de", 
	"fi", 
	"fr", 
	"gr", 
	"nl", 
	"hu", 
	"it", 
	"no", 
	"pl", 
	"pt", 
	"ru", 
	"ro", 
	"es", 
	"se", 
	"tw", 
	"tr", 
	"cn", 
	"ua"
};

class SteamGameCrawler {
	public :
		SteamGameCrawler() {
			dbConn = new MySQLConnector();
			curl = new CurlConnector();
			
			this->seedURL = "";
		};
		SteamGameCrawler(string seedURL, string ip, int port, string id, string pwd, string db) {
			dbConn = new MySQLConnector(ip, port, id, pwd, db);
			curl = new CurlConnector();
			this->seedURL = seedURL;
		};
		
		bool run();
	private :
		MySQLConnector *dbConn;
		CurlConnector *curl;
		
		struct timeval start, end;
		long calTime();

		string seedURL;
};

#endif /* STEAMGAMECRAWLER_H */
