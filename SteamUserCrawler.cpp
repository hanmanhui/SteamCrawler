#include "SteamUserCrawler.h"


bool SteamUserCrawler::run() {
    // Connect To DB
    dbConn->connect();
    
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;
    
    string page;
    
    string userName = "";
    int userLevel = -1;
    
    while(!friends.empty()) {
        string url = friends.front();
        friends.pop();
        
        string page = curl->getPage(url);
        
        string userName = "";
        int userLevel = -1;
        
        // Getting User Name & User Steam Level
        if(page != "") {
            GumboOutput *output = gumbo_parse(page.c_str());
            
            queue<GumboNode *> nodes;
            nodes.push(output->root);
            
            while(!nodes.empty() && (userName != "" && userLevel != -1)) {
                GumboNode *node = nodes.front();
                nodes.pop();
                
                if(node->type != GUMBO_NODE_ELEMENT) {
                    continue;
                }
                
                GumboAttribute *attr;
                
                // User Name
                if((node->v.element.tag == GUMBO_TAG_DIV) &&
                (attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
                (strcmp(attr->value, "persona_name"))) {
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
                (strcmp(attr->value, "persona_name persona_level"))) {
                    GumboVector *aChild = &node->v.element.children;
                    for(size_t i = 0; i < aChild.length; i++) {
                        GumboNode *aNode = static_cast<GumboNode *>(aChild->data[i]);
                        if((aNode->type == GUMBO_NODE_ELEMENT) &&
                        (aNode->v.element.tag == GUMBO_TAG_DIV)) {
                            GumboVector *bChild = &aNode->v.element.children;
                            for(size_t j = 0; j < bChild.length; j++) {
                                GumboNode *bNode = static_cast<GumboNode *>(bChild->data[i]);
                                if((bNode->type == GUMBO_NODE_ELEMENT) &&
                                (bNode->v.element.tag == GUMBO_TAG_SPAN)) {
                                    GumboVector *cChild = &bNode->v.element.children;
                                    for(size_t k = 0; k < cChild->length; k++) {
                                        GumboNode *cNode = static_cast<GumboNode *>(cChild->data[i]);
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
            
            // Save to DB
            stmt = dbConn->con->createStatement();
            string q = "INSERT INTO user(url, name, steamlv) VALUES(" + 
                + "'" + url + "', "
                + "'" + userName + "', "
                + steamlv + ")"
                + "ON DUPLICATE KEY UPDATE" 
                + "name = VALUES(name)" +
                + "steamlv = VALUES(steamlv);"
            stmt->execute(q.c_str());
            delete stmt;
        }
        
        string friendListUrl = url + "/friends";
        
        page = curl->getPage(friendListUrl);
        
        if(page != "") {
            GumboOutput *output = gumbo_parse(page.c_str());
            
            if(output->root->type == GUMBO_NODE_ELEMENT) {
                queue<GumboNode *> nodes;
                nodes.push(output->root);
                
                while(!nodes.empty()) {
                    GumboNode *node = nodes.front();
                    nodes.pop();
                    
                    if(node->type != GUMBO_NODE_ELEMENT) {
                        continue;
                    }
                    
                    GumboAttribute *attr;
                    if((node->v.element.tag == GUMBO_TAG_A) &&
                    (attr = gumbo_getattribute(&node->v.element.attributes, "class")) &&
                    (strcmp(attr->value, "friendBlockLinkOverlay") == 0) && 
                    (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
                        string friendURL = attr->value;
                        
                        stmt = dbConn->con->createStatement();
                        try {
                            string q = "INSERT INTO user(url) VALUES("
                            + "'" + friendURL + "');"
                            stmt->exequte(q.c_str());
                            
                            friends.push(friendURL);
                        } catch(sql::SQLException &e) {
                            if(friends.size() <= MAX_QUEUE_SIZE) {
                                friends.push(friendURL);
                            }
                        }
                        
                        try {
                            string q = "INSERT INTO friends(user_id, friend_id) VALUES("
                            + "(SELECT id"
                            + "FROM user"
                            + "where url = '" + url + "'), "
                            + "(SELECT id"
                            + "FROM user"
                            + "where url = '" + friendUrl + "'));"
                            stmt->execute(q.c_str());
                        } catch(sql::SQLException &e) {
                            cout << "SQL Error on inserting Friends" << endl;
                        }
                        
                        delete stmt;
                    }
                }
                
                GumboVector *children = &node->v.element.children;
        		for(size_t i = 0; i < children->length; i++) {
        			nodes.push(static_cast<GumboNode *>(children->data[i]));
        		}
            }
            
            gumbo_destroy_output(&kGumboDefaultOptions, output);
        }
    }
    
    cout << "No more new friends." << endl
        << "Input another seed." << endl;
        
    return false;
}
