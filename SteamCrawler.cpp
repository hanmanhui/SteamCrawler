#include <iostream>

using namespace std;

#include "MySQLConnector.h"
#include "SteamUserCrawler.h"

int main(int argc, char** argv) {
    if(argc != 5) {
        cout << "Usage : SteamCrawler MySQL_IP Port MySQL_ID MySQL_PassWD MySQL_DB" << endl;
    }
    
    string ip = argv[1];
    int port = argv[2];
    string id = argv[3];
    string pwd = argv[4];
    string db = argv[5];
	
	// Prepare DB
	MySQLConnector conn(ip, port, id, pwd, db);
	
	conn->connect();
	
	sql::Statement *stmt;

    stmt = conn->con->createStatement();
    stmt->execute("CREATE TABLE IF NOT EXISTS user(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, "
     + "url VARCHAR(255) NOT NULL, " 
     + "name VARCHAR(255), "
     + "games INT, "
     + "steamlv INT, "
     + "UNIQUE(url));"
    );
    stmt->execute("CREATE TABLE IF NOT EXISTS game(id INT PRIMARY KEY AUTO_INCREMENT NOT NULL, "
     + "url VARCHAR(255) NOT NULL, "
     + "title VARCHAR(255) NOT NULL, "
     + "release_date VARCHAR(255), "
     + "metascore INT, "
     + "genre VARCHAR(255), "
     + "developer VARCHAR(255), "
     + "publisher VARCHAR(255), "
     + "UNIQUE(url));"
    );
    stmt->execute("CREATE TABLE IF NOT EXISTS user_game (user_id INT NOT NULL, "
     + "game_id INT NOT NULL, "
     + "played FLOAT, "
     + "FOREIGN KEY (user_id) REFERENCES user(id), "
     + "FOREIGN KEY (game_id) REFERENCES game(id));"
    );
    stmt->execute("CREATE TABLE IF NOT EXISTS friends (user_id INT NOT NULL, "
     + "friend_id INT NOT NULL, "
     + "FOREIGN KEY (user_id) REFERENCES user(id), "
     + "FOREIGN KEY (friend_id) REFERENCES user(id));"
    );
    delete stmt;
    
    // Init Crawler
    SteamUserCrawler userCrawler(ip, port, id, pwd, db);
    
    // Run Crawler
    userCrawler->rusn();
	
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