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
    
    while(!userURL.empty()) {
        printf("Current URL Queue Size : %ld\n", userURL.size());
        
        gettimeofday(&start, NULL);
        string url = userURL.front();
        userURL.pop();
        userURLRef.erase(url);
        gettimeofday(&end, NULL);
        printf("Getting User URL from Queue Done (time consumed : %ldms)\n", this->calTime());
        
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
                        try {
                            string q = "INSERT INTO user(url) VALUES(";
                            q += "'" + friendURL + "');";
                            stmt->execute(q.c_str());
                            
                            if(userURLRef.find(friendURL) == userURLRef.end()) {
                                userURL.push(friendURL);
                                userURLRef.insert(friendURL);
                            }
                        } catch(sql::SQLException &e) {
                            if(userURL.size() <= MAX_QUEUE_SIZE) {
                                if(userURLRef.find(friendURL) == userURLRef.end()) {
                                    userURL.push(friendURL);
                                    userURLRef.insert(friendURL);
                    	        }
                            }
                        }
                        
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

		stmt = dbConn->con->createStatement();

		gettimeofday(&start, NULL);
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
    }
    delete pstmt;

    printf("No more new friends.\nInput another seed.\n");

    return false;
}
