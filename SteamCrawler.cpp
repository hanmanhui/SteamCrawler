#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

#include "MySQLConnector.h"
#include "SteamUserCrawler.h"

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

	string q = "CREATE TABLE IF NOT EXISTS user(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, ";
	q += "url VARCHAR(255) NOT NULL, ";
	q += "name VARCHAR(255), ";
	q += "games INT, ";
	q += "steamlv INT, ";
	q += "UNIQUE(url));";

	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS game(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, ";
	q += "url VARCHAR(255), ";
	q += "title VARCHAR(255) NOT NULL, ";
	q += "release_date VARCHAR(255), ";
	q += "metascore INT, ";
	q += "genre VARCHAR(255), ";
	q += "developer VARCHAR(255), ";
	q += "publisher VARCHAR(255), ";
	q += "UNIQUE(title));";
    
	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS user_game (user_id INT NOT NULL, ";
	q += "game_id INT NOT NULL, ";
	q += "played FLOAT, ";
	q += "FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE, ";
	q += "FOREIGN KEY (game_id) REFERENCES game(id) ON DELETE CASCADE ON UPDATE CASCADE);";

	stmt->execute(q.c_str());

	q = "CREATE TABLE IF NOT EXISTS friends (user_id INT NOT NULL, ";
	q += "friend_id INT NOT NULL, ";
	q += "FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE, ";
	q += "FOREIGN KEY (friend_id) REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE);";

	stmt->execute(q.c_str());

	delete stmt;

    string seedURL;
    printf("Enter the Seed URL for Crawling\n");
    cin >> seedURL;
	// Init Crawler
	SteamUserCrawler userCrawler(seedURL, ip, port, id, pwd, db);
    
	// Run Crawler
	userCrawler.run();
	
	return 0;
}

/*
static void GumboSelector(vector<GumboNode *> &result, string selector, GumboNode *root) {
	string temp = "";
	for(size_t i = 0; i < selector.length(); i++) {
		if(delimiters.find(selector[i]) != string::npos) {
			if(temp != "") {
				tokens.push_back(temp);
			}
			temp = selector[i];
			tokens.push_back(temp);
			temp = "";
		} else {
			temp += selector[i];
		}			
	}
	if(temp != "") {
		tokens.push_back(temp);
	}

	int cur = 0;
	queue<GumboNode *> nodes;
	nodes.push(root);

	while(!nodes.empty()) {
		GumboNode *node = nodes.front();
		nodes.pop();

		if(node->type != GUMBO_NODE_ELEMENT) {
			return;
		}

		GumboVector *children = &node->v.element.children;
		for(size_t i = 0; i < children->length; i++) {
			nodes.push(static_cast<GumboNode *>(children->data[i]));
		}
	
		for(size_t i = 0; i < tokens.size(); i++) {
			cout << tokens[i] << endl;
		}
	}
}
*/
