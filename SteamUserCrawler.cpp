#include "SteamUserCrawler.h"

long SteamUserCrawler::calTime() {
	long seconds = this->end.tv_sec - this->start.tv_sec;
	long useconds = this->end.tv_usec - this->start.tv_usec;
	
	return ((seconds)*1000 + useconds/1000.0) + 0.5;
}

bool SteamUserCrawler::run() {
	printf("Started Running User Crawler\n");
	
	gettimeofday(&start, NULL);
	// Connect To DB
	dbConn->connect();
	gettimeofday(&end, NULL);
	printf("DB Connected (time consumed : %ldms)\n", this->calTime());

	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *pstmt;
	pstmt = dbConn->con->prepareStatement("INSERT INTO user(url, name, steamlv) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE name=VALUES(name), steamlv=VALUES(steamlv);");   

	string page;
	
	string userName;
	int userLevel;

    string url = this->seedURL;
    if(url == "") {
	    gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM user WHERE name IS NULL ORDER BY RAND() LIMIT 1;");
		gettimeofday(&end, NULL);
		if(res->next()) {
			url = res->getString(1);
			printf("Getting Random Seed URL from DB Done (time consumed : %ld)\n", this->calTime());
		}
		delete stmt;
		delete res;
    }
    
	while(url != "") {
		gettimeofday(&start, NULL);
		string page = curl->getPage(url);
		gettimeofday(&end, NULL);
		printf("Getting User Profile Page Done (time consumed : %ldms)\n", this->calTime());
		
		userName = "";
		userLevel = -1;

		// Getting User Name & User Steam Level
		if(page != "") {
			gettimeofday(&start, NULL);
			GumboOutput *output = gumbo_parse(page.c_str());
			gettimeofday(&end, NULL);
			printf("User Profile Page Parsing Done (time consumed : %ldms)\n", this->calTime());
			printf("Current URL : [%s]\n", url.c_str());
			
			queue<GumboNode *> nodes;
			nodes.push(output->root);
			
			gettimeofday(&start, NULL);
			while(!nodes.empty() && (userName == "" || userLevel == -1)) {
				GumboNode *node = nodes.front();
				nodes.pop();
				
				if(node->type != GUMBO_NODE_ELEMENT) {
					continue;
				}
				
				GumboAttribute *attr;
	
				// User Name
				if((node->v.element.tag == GUMBO_TAG_DIV) &&
				(attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
				(strcmp(attr->value, "persona_name") == 0)) {
					GumboVector *aChild = &node->v.element.children;
					for(size_t i = 0; i < aChild->length; i++) {
						GumboNode *aNode = static_cast<GumboNode *>(aChild->data[i]);
						if(aNode->type == GUMBO_NODE_TEXT) {
							userName = aNode->v.text.text;
							trim(userName);
						}
					}
					continue;
				}
	
				// User Steam Level
				if((node->v.element.tag == GUMBO_TAG_DIV) &&
				(attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
				(strcmp(attr->value, "persona_name persona_level") == 0)) {
					GumboVector *aChild = &node->v.element.children;
					for(size_t i = 0; i < aChild->length; i++) {
						GumboNode *aNode = static_cast<GumboNode *>(aChild->data[i]);
						if((aNode->type == GUMBO_NODE_ELEMENT) &&
						(aNode->v.element.tag == GUMBO_TAG_DIV)) {
							GumboVector *bChild = &aNode->v.element.children;
							for(size_t j = 0; j < bChild->length; j++) {
								GumboNode *bNode = static_cast<GumboNode *>(bChild->data[j]);
								if((bNode->type == GUMBO_NODE_ELEMENT) &&
								(bNode->v.element.tag == GUMBO_TAG_SPAN)) {
									GumboVector *cChild = &bNode->v.element.children;
									for(size_t k = 0; k < cChild->length; k++) {
										GumboNode *cNode = static_cast<GumboNode *>(cChild->data[k]);
										if(cNode->type == GUMBO_NODE_TEXT) {
											stringstream ss;
											ss << cNode->v.text.text;
											ss >> userLevel;
										}
									}
								}
							}
						}
					}
				}
					
				GumboVector *children = &node->v.element.children;
				for(size_t i = 0; i < children->length; i++) {
					nodes.push(static_cast<GumboNode *>(children->data[i]));
				}
			}
			gumbo_destroy_output(&kGumboDefaultOptions, output);
			gettimeofday(&end, NULL);
			printf("Searching User Profile Information Done (time consumed : %ldms)\n", this->calTime());
		}

		gettimeofday(&start, NULL);
		// Save to DB
		pstmt->setString(1, url);
		if(userName != "") {
			pstmt->setString(2, userName);
		} else {
			pstmt->setNull(2, 0);
		}
		if(userLevel != -1) {
			pstmt->setInt(3, userLevel);
		} else {
			pstmt->setNull(3, 0);
		}
		pstmt->execute();
		gettimeofday(&end, NULL);
		printf("Saving User Profile to Database Done (time consumed : %ldms)\n", this->calTime());

		string friendListUrl = url + "/friends";
		
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
						
						try {
							string q = "INSERT IGNORE INTO friends(user_id, friend_id) VALUES(";
							q += "(SELECT id ";
							q += "FROM user ";
							q += "where url = '" + url + "'), ";
							q += "(SELECT id ";
							q += "FROM user ";
							q += "where url = '" + friendURL + "'));";
							stmt->execute(q.c_str());
						} catch(sql::SQLException &e) {
							printf("SQL Error on inserting Friends\n");
						}
					
						delete stmt;
					}
				
					GumboVector *children = &node->v.element.children;
					for(size_t i = 0; i < children->length; i++) {
						nodes.push(static_cast<GumboNode *>(children->data[i]));
					}
				}
				gettimeofday(&end, NULL);
				printf("Getting & Saving User's Friends Information Done (time consumed : %ldms)\n", this->calTime());

				gettimeofday(&start, NULL);
				stmt = dbConn->con->createStatement();
				
				string q = "UPDATE user SET friends=(";
				q += "(SELECT count(DISTINCT friend_id) ";
				q += "FROM friends WHERE user_id=";
				q += "(SELECT id FROM (SELECT * FROM user) AS tUser ";
				q += "WHERE url='";
				q += url;
				q += "'))) WHERE url='";
				q += url;
				q += "';";
				stmt->execute(q.c_str());
				printf("Saving User's Friends Count Done (time consumed : %ldms)\n", this->calTime());
				gettimeofday(&end, NULL);

				delete stmt;
			}
			
			gumbo_destroy_output(&kGumboDefaultOptions, output);
		}
			
		string userGameUrl = url + "/games?tab=all";
		gettimeofday(&start, NULL);
		page = curl->getPage(userGameUrl);
		gettimeofday(&end, NULL);
		printf("Getting User's Games Page Done (time consumed : %ldms)\n", this->calTime());
		
		if(page != "") {
			gettimeofday(&start, NULL);
			GumboOutput *output = gumbo_parse(page.c_str());
			gettimeofday(&end, NULL);
			printf("Parsing User's Games Page Done (time consumed : %ldms)\n", this->calTime());
			
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
					if((node->v.element.tag == GUMBO_TAG_DIV) &&
					(attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
					(strcmp(attr->value, "gameListRow") == 0) && 
					(attr = gumbo_get_attribute(&node->v.element.attributes, "id"))) {
						string gameID = attr->value;
						string gameName = "";
						float gamePlayed = -1;
						
						GumboVector *aChild = &node->v.element.children;
						for(size_t i = 0; i < aChild->length; i++) {
						    GumboNode *aNode = static_cast<GumboNode *>(aChild->data[i]);
						    
						    if((aNode->type == GUMBO_NODE_ELEMENT) &&
						    (aNode->v.element.tag == GUMBO_TAG_DIV) &&
						    (attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
						    (strcmp(attr->value, "gameListRowItem") == 0)) {
						        GumboVector *bChild = &aNode->v.element.children;
						        for(size_t j = 0; j < bChild->length; j++) {
						            GumboNode *bNode = static_cast<GumboNode *>(bChild->data[j]);
						            if((bNode->type == GUMBO_NODE_ELEMENT) &&
						            (bNode->v.element.tag == GUMBO_TAG_DIV) &&
						            (attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
						            (strcmp(attr->value, "gameListRowItemName")) {
						                GumboVector *cChild = &bNode->v.element.children;
						                for(size_t k = 0; k < cChild->length; k++) {
						                    GumboNode *cNode = static_cast<GumboNode *>(cChild->data[k]);
						                    if(cNode->type == GUMBO_NODE_TEXT) {
						                        gameName = cNode->v.text.text;
						                    }
						                }
						            } else if((bNode->type == GUMBO_NODE_ELEMENT) &&
						            (bNode->v.element.tag == GUMBO_TAG_H5)) {
						                GumboVector *cChild = &bNode->v.element.children;
						                for(size_t k = 0; k < cChild->length; k++) {
						                    GumboNode *cNode = static_cast<GumboNode *>(cChild->data[k]);
						                    if(cNode->type == GUMBO_NODE_TEXT) {
						                        stringstream ss;
						                        ss << cNode->v.text.text;
						                        ss >> gamePlayed;
						                    }
						                }
						            }
						        }
						    }
						}
						
						if(gameName != "") {
				            stmt = dbConn->con->createStatement();
				            string q = "INSERT IGNORE INTO game(title) VALUES('";
				            
				            std::ostringstream out;
				            for(size_t i = 0; i < gameName.size(); i++) {
				                switch(gameName[i]) {
				                    case : "'" {
				                        out << "\";
				                        break;
				                    }
				                }
				                
				                out << gameName[i];
				            }
				            q += out.str();
				            q += "');"
				            stmt->execute(q.c_str());
				            
				            if(gamePlayed != -1) {
				                q = "INSERT INTO user_game(user_id, game_id, played) VALUES(";
				                q += "(SELECT id ";
				                q += "FROM user ";
				                q += "WHERE url = '" + url + "'), ";
				                q += "(SELECT id ";
				                q += "FROM game ";
				                q += "WHERE title = '" + out.str() + "'), ";
				                q += gamePlayed;
				                q += ");"
				            } else {
				                q = "INSERT INTO user_game(user_id, game_id) VALUES(";
				                q += "(SELECT id ";
				                q += "FROM user ";
				                q += "WHERE url = '" + url + "'), ";
				                q += "(SELECT id ";
				                q += "FROM game ";
				                q += "WHERE title = '" + out.str() + "'));";
				            }
				            stmt->execute(q.c_str());
						}
						
						delete stmt;
					}
				
					GumboVector *children = &node->v.element.children;
					for(size_t i = 0; i < children->length; i++) {
						nodes.push(static_cast<GumboNode *>(children->data[i]));
					}
				}
				gettimeofday(&end, NULL);
				printf("Getting & Saving User's Games Information Done (time consumed : %ldms)\n", this->calTime());

				gettimeofday(&start, NULL);
				stmt = dbConn->con->createStatement();
				
				string q = "UPDATE user SET games=(";
				q += "(SELECT count(DISTINCT game_id) ";
				q += "FROM user_game WHERE user_id=";
				q += "(SELECT id FROM (SELECT * FROM user) AS tUser ";
				q += "WHERE url='";
				q += url;
				q += "'))) WHERE url='";
				q += url;
				q += "';";
				stmt->execute(q.c_str());
				printf("Saving User's Games Count Done (time consumed : %ldms)\n", this->calTime());
				gettimeofday(&end, NULL);

				delete stmt;
			}
		}

		// Getting Next Seed Url
		url = "";
		gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT url FROM user WHERE name IS NULL ORDER BY RAND() LIMIT 1;");
		gettimeofday(&end, NULL);
		if(res->next()) {
			url = res->getString(1);
			printf("Getting Random URL from DB Done (time consumed : %ld)\n", this->calTime());
		}
		delete stmt;
		delete res;
	}
	delete pstmt;

	printf("No more new friends.\nInput another seed.\n");

	return false;
}
