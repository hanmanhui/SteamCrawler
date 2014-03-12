#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

// Boost Thread
#include <boost/thread.hpp>

#include "MySQLConnector.h"
#include "SteamUserCrawler.h"
#include "SteamFriendsCrawler.h"
#include "SteamUserGameCrawler.h"

int main(int argc, char** argv) {
	if(argc != 6) {
		cout << "Usage : SteamCrawler MySQL_IP Port MySQL_ID MySQL_PassWD MySQL_DB" << endl;
		return -1;
	}
    
	string ip = argv[1];
	int port = atoi(argv[2]);
	string id = argv[3];
	string pwd = argv[4];
	string db = argv[5];
	
	// Prepare DB
	MySQLConnector conn(ip, port, id, pwd, db);
	
	conn.connect();
	
	sql::Statement *stmt;

	stmt = conn.con->createStatement();

	string q = "CREATE TABLE IF NOT EXISTS user(";
	q += "id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, ";
	q += "url VARCHAR(255) NOT NULL, ";
	q += "name VARCHAR(255), ";
	q += "games INT, ";
	q += "friends INT, ";
	q += "steamlv INT, ";
	q += "UNIQUE(url));";

	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS game(";
	q += "appid INT PRIMARY KEY NOT NULL, ";
	q += "url VARCHAR(255), ";
	q += "title VARCHAR(255) NOT NULL, ";
	q += "release_date VARCHAR(255), ";
	q += "metascore INT, ";
	q += "genre VARCHAR(255), ";
	q += "developer VARCHAR(255), ";
	q += "publisher VARCHAR(255), ";
	q += "UNIQUE(appid));";
    
	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS user_game (";
	q += "user_id INT NOT NULL, ";
	q += "game_id INT NOT NULL, ";
	q += "played FLOAT, ";
	q += "PRIMARY KEY (user_id, game_id), ";
	q += "FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE, ";
	q += "FOREIGN KEY (game_id) REFERENCES game(appid) ON DELETE CASCADE ON UPDATE CASCADE);";

	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS friends (";
	q += "user_id INT NOT NULL, ";
	q += "friend_id INT NOT NULL, ";
	q += "PRIMARY KEY (user_id, friend_id), ";
	q += "FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE, ";
	q += "FOREIGN KEY (friend_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE);";

	stmt->execute(q.c_str());

	delete stmt;

	// Init Crawler
	SteamUserCrawler *userCrawler;
	SteamFriendsCrawler *friendsCrawler;
	SteamUserGameCrawler *userGameCrawler;

	// Init Thread
	boost::thread *t1;
	boost::thread *t2;
	boost::thread *t3;

	while(1) {
		string seedURL;
		cout << "Enter the Seed URL for Crawling" << endl;
		cout << "Type 'r' for Random URL Seed" << endl;
		cin >> seedURL;
 
		userCrawler = new SteamUserCrawler(seedURL, ip, port, id, pwd, db);
		friendsCrawler = new SteamFriendsCrawler(seedURL, ip, port, id, pwd, db);
		userGameCrawler = new SteamUserGameCrawler(seedURL, ip, port, id, pwd, db);
	    
		t1 = new boost::thread(boost::bind(&SteamUserCrawler::run, userCrawler));
		t2 = new boost::thread(boost::bind(&SteamFriendsCrawler::run, friendsCrawler));
		t3 = new boost::thread(boost::bind(&SteamUserGameCrawler::run, userGameCrawler));

		t1->join();
		t2->join();
		t3->join();

		cout << "No new things to crawl" << endl;
	}

	return 0;
}
