#include "SteamUrlCrawler.h"

long SteamUrlCrawler::calTime() {
	long seconds = this->end.tv_sec - this->start.tv_sec;
	long useconds = this->end.tv_usec - this->start.tv_usec;
	
	return ((seconds)*1000 + useconds/1000.0) + 0.5;
}

bool SteamUrlCrawler::run() {
	printf("Started Running Url Crawler\n");
	
	gettimeofday(&start, NULL);
	// Connect To DB
	dbConn->connect();
	gettimeofday(&end, NULL);
	printf("DB Connected (time consumed : %ldms)\n", this->calTime());

	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *pstmt;
	pstmt = dbConn->con->prepareStatement("INSERT INTO url(url) VALUES (?) ON DUPLICATE KEY UPDATE timestamp=CURRENT_TIMESTAMP;");   
	string page;
	queue<string> q;

    string url = this->seedURL;
    if(url == "r") {
	    gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM url WHERE timestamp=(SELECT MIN(TIMESTAMP) FROM url);");
		gettimeofday(&end, NULL);
		while(res->next()) {
			q.push(res->getString(1));
			printf("Getting URLs from DB Done (time consumed : %ldms)\n", this->calTime());
		}
		delete stmt;
		delete res;
    } else {
		q.push(seedURL);
	}
    
	while(!q.empty()) {
		gettimeofday(&start, NULL);
		url = q.front();
		q.pop();
		string page = curl->getPage(url);
		gettimeofday(&end, NULL);
		printf("Getting Page Done (time consumed : %ldms)\n", this->calTime());
		
		if(page != "") {
			gettimeofday(&start, NULL);
			GumboOutput *output = gumbo_parse(page.c_str());
			gettimeofday(&end, NULL);
			printf("Page Parsing Done (time consumed : %ldms)\n", this->calTime());
			printf("Current Url [%s]\n", url.c_str());
			
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
				(attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) { 
					string t = attr->value;
					if(t.find("http") != string::npos) {
						pstmt->setString(1, attr->value);
						pstmt->executeUpdate();
					}
				}

				GumboVector *children = &node->v.element.children;
				for(size_t i = 0; i < children->length; i++) {
					nodes.push(static_cast<GumboNode *>(children->data[i]));
				}
			}
			gumbo_destroy_output(&kGumboDefaultOptions, output);
			gettimeofday(&end, NULL);
			printf("Saving Urls to Database Done (time consumed : %ldms)\n", this->calTime());
		}

		// Getting Next Seed Url
		url = "";
		gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM url WHERE timestamp=(SELECT MIN(TIMESTAMP) FROM url);");

		while(res->next()) {
			q.push(res->getString(1));
		}
		gettimeofday(&end, NULL);
		printf("Getting Next Urls Done (time consumed : %ldms)\n", this->calTime());

		delete stmt;
		delete res;
	}
	delete pstmt;

	return false;
}
