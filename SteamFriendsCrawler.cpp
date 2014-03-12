#include "SteamFriendsCrawler.h"

long SteamFriendsCrawler::calTime() {
	long seconds = this->end.tv_sec - this->start.tv_sec;
	long useconds = this->end.tv_usec - this->start.tv_usec;
	
	return ((seconds)*1000 + useconds/1000.0) + 0.5;
}

bool SteamFriendsCrawler::run() {
	printf("Started Running User Crawler\n");
	
	gettimeofday(&start, NULL);
	// Connect To DB
	dbConn->connect();
	gettimeofday(&end, NULL);
	printf("DB Connected (time consumed : %ldms)\n", this->calTime());

	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *pstmt;
	pstmt = dbConn->con->prepareStatement("UPDATE user SET friends=((SELECT count(DISTINCT friend_id) FROM friends WHERE user_id=?)) WHERE url=?;");
	string page;
	
	string url = this->seedURL;

	if(url == "r") {
		gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM user WHERE friends IS NULL ORDER BY RAND() LIMIT 1;");
		gettimeofday(&end, NULL);
		if(res->next()) {
			url = res->getString(1);
			printf("Getting Random Seed URL from DB Done (time consumed : %ldms)\n", this->calTime());
		}
		delete stmt;
		delete res;
	}
    
	while(url != "") {
		string friendListUrl = url + "/friends";
		printf("Current URL [%s]\n", friendListUrl.c_str());

		gettimeofday(&start, NULL);
		page = curl->getPage(friendListUrl);
		gettimeofday(&end, NULL);
		printf("Getting User's Friends Page Done (time consumed : %ldms)\n", this->calTime());
		
		if(page != "") {
			gettimeofday(&start, NULL);
			GumboOutput *output = gumbo_parse(page.c_str());
			gettimeofday(&end, NULL);
			printf("Parsing User's Friends Page Done (time consumed : %ldms)\n", this->calTime());
			
			if(output->root->type == GUMBO_NODE_ELEMENT) {
				queue<GumboNode *> nodes;
				nodes.push(output->root);

				gettimeofday(&start, NULL);	
				while(!nodes.empty()) {
					GumboNode *node = nodes.front();
					nodes.pop();
					
					if(node->type != GUMBO_NODE_ELEMENT) {
						continue;
					}
					
					GumboAttribute *attr;
					if((node->v.element.tag == GUMBO_TAG_A) &&
					(attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
					(strcmp(attr->value, "friendBlockLinkOverlay") == 0) && 
					(attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
						string friendURL = attr->value;
						
						stmt = dbConn->con->createStatement();
						string q = "INSERT IGNORE INTO user(url) VALUES(";
						q += "'" + friendURL + "');";
						stmt->execute(q.c_str());
						
						q = "INSERT IGNORE INTO friends(user_id, friend_id) VALUES(";
						q += "(SELECT id ";
						q += "FROM user ";
						q += "where url = '" + url + "'), ";
						q += "(SELECT id ";
						q += "FROM user ";
						q += "where url = '" + friendURL + "'));";
						stmt->execute(q.c_str());
					
						delete stmt;
					}
				
					GumboVector *children = &node->v.element.children;
					for(size_t i = 0; i < children->length; i++) {
						nodes.push(static_cast<GumboNode *>(children->data[i]));
					}
				}
				gettimeofday(&end, NULL);
				printf("Getting & Saving User's Friends Information Done (time consumed : %ldms)\n", this->calTime());

				// Save Friends Count to user
				gettimeofday(&start, NULL);
				stmt = dbConn->con->createStatement();
				string q = "SELECT id FROM user WHERE url='";
				q += url;
				q += "' LIMIT 1;";

				res = stmt->executeQuery(q.c_str());
				if(res->next()) {
					pstmt->setInt(1, res->getInt(1));
					pstmt->setString(2, url);
					pstmt->execute();
					gettimeofday(&end, NULL);
					printf("Saving User's Friends Count Done (time consumed : %ldms)\n", this->calTime());
				}
			}
			
			gumbo_destroy_output(&kGumboDefaultOptions, output);
		}

		// Getting Next Seed Url
		url = "";
		gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM user WHERE friends IS NULL ORDER BY RAND() LIMIT 1;");
		gettimeofday(&end, NULL);
		if(res->next()) {
			url = res->getString(1);
			printf("Getting Random URL from DB Done (time consumed : %ldms)\n", this->calTime());
		}
		delete stmt;
		delete res;
	}
	delete pstmt;

	return false;
}
