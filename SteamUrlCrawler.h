#ifndef STEAMURLCRAWLER_H
#define STEAMURLCRAWLER_H

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

class SteamUrlCrawler {
	public :
		SteamUrlCrawler() {
			dbConn = new MySQLConnector();
			curl = new CurlConnector();
			
			this->seedURL = "";
		};
		SteamUrlCrawler(string seedURL, string ip, int port, string id, string pwd, string db) {
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

#endif /* STEAMURLCRAWLER_H */
