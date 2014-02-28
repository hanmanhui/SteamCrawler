#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <gumbo.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;

queue<string> friends;

size_t curlToString(void *ptr, size_t size, size_t nmemb, void *data) {
	string *str = (string *)data;
	char *sptr = (char *) ptr;

	for(size_t i = 0; i < size*nmemb; i++) {
		(*str) += sptr[i];
	}

	return size*nmemb;
}

GumboVector* GetGumboChildren(GumboNode *node, GumboTag tag) {
	if((node->v.element.tag == tag)) {
		return &node->v.element.children;
	} else {
		return NULL;
	}
}

GumboVector* GetGumboChildren(GumboNode *node, GumboTag tag, const char* attribute, const char* value) {
	GumboAttribute *attr;
	if((node->v.element.tag == tag) &&
	   (attr = gumbo_get_attribute(&node->v.element.attributes, attribute)) && 
	   (strcmp(attr->value, value) == 0)) {
		return &node->v.element.children;
	} else {
		return NULL;
	}
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
static void searchForPersonalProfile(string userUrl, GumboNode* root) {
	// Searching Things
	//	userName
	//	userLevel
	//	userGames
	string userGameUrl = userUrl + "/games/";

	string userName = "";
	int userLevel = -1;
	int userGames = -1;
	
	if(root->type != GUMBO_NODE_ELEMENT) {
		return;
	}

	queue<GumboNode *> nodes;
	nodes.push(root);

	while(!nodes.empty()) {
		GumboNode *node = nodes.front();
		nodes.pop();

		if(node->type != GUMBO_NODE_ELEMENT) {
			continue;
		}
		
		//	userName
		GumboVector *children;
		if((userName == "") &&
		   (children = GetGumboChildren(node, GUMBO_TAG_DIV, "class", "persona_name"))) {
			for(size_t i = 0; i < children->length; i++) {
				GumboNode *n = static_cast<GumboNode *>(children->data[i]);
				if(n->type == GUMBO_NODE_TEXT) {
					userName = n->v.text.text;
					trim(userName);
				}
			}
			continue;
		}

		//	userLevel
		if((userLevel == -1) &&
		   (children = GetGumboChildren(node, GUMBO_TAG_DIV, "class", "persona_name persona_level"))) {
			for(size_t i = 0; i < children->length; i++) {
				GumboNode *aNode = static_cast<GumboNode *>(children->data[i]);
				GumboVector *aChild = GetGumboChildren(aNode, GUMBO_TAG_DIV);

				if(aChild) {		
					for(size_t j = 0; j < aChild->length; j++) {
						GumboNode *bNode = static_cast<GumboNode *>(aChild->data[j]);
						GumboVector *bChild = GetGumboChildren(bNode, GUMBO_TAG_SPAN);
						if(bChild) {
							for(size_t k = 0; k < bChild->length; k++) {
								GumboNode *cNode = static_cast<GumboNode *>(bChild->data[k]);
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

		//	userGames
		if((userGames == -1) && 
		   (children = GetGumboChildren(node, GUMBO_TAG_A, "href", userGameUrl.c_str()))) {
			for(size_t i = 0; i < children->length; i++) {
				GumboNode *aNode = static_cast<GumboNode *>(children->data[i]);
				GumboVector *aChild;
				if((aChild = GetGumboChildren(aNode, GUMBO_TAG_SPAN, "class", "profile_count_link_total"))) {
					for(size_t j = 0; j < aChild->length; j++) {
						GumboNode *bNode = static_cast<GumboNode *>(aChild->data[j]);
						if(bNode->type == GUMBO_NODE_TEXT) {
							stringstream ss;
							ss << bNode->v.text.text;
							ss >> userGames;
						}
					}
				}
			}
			continue;
		}

		children = &node->v.element.children;
		for(size_t i = 0; i < children->length; i++) {
			nodes.push(static_cast<GumboNode *>(children->data[i]));
		}
	}

	cout << "=======================================================" << endl
		<< "UserUrl : " << userUrl << endl
		<< "UserName : " << userName << endl
		<< "UserLevel : " << userLevel << endl
		<< "UserGames : " << userGames << endl
		<< "=======================================================" << endl << endl;
}

static void CollectFriends(GumboNode* root) {
	if(root->type != GUMBO_NODE_ELEMENT) {
		return;
	}

	queue<GumboNode *> nodes;
	nodes.push(root);

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
			friends.push(attr->value);
		}

		GumboVector *children = &node->v.element.children;
		for(size_t i = 0; i < children->length; i++) {
			nodes.push(static_cast<GumboNode *>(children->data[i]));
		}
	}
}

int main(int argc, char** argv) {
	if(argc != 2) {
		cout << "Usage : Crawler 'Url'" << endl << endl;
		return -1;
	}

	friends.push(argv[1]);
	string page;

	CURL *curl;
	CURLcode res;

	while(!friends.empty()) {
		string url = friends.front();
		friends.pop();

		curl = curl_easy_init();
		if(curl) {
			page = "";

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page);

			res = curl_easy_perform(curl);
			if(res != CURLE_OK)
				cout << "Error : curl_easy_perform() failed : " << curl_easy_strerror(res) << endl;
			curl_easy_cleanup(curl);

			GumboOutput *output = gumbo_parse(page.c_str());
			searchForPersonalProfile(url, output->root);
			gumbo_destroy_output(&kGumboDefaultOptions, output);
		}

		curl = curl_easy_init();
		if(curl) {
			string friendListUrl = url + "/friends/";
			page = "";

			curl_easy_setopt(curl, CURLOPT_URL, friendListUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page);

			res = curl_easy_perform(curl);
			if(res != CURLE_OK)
				cout << "Error : curl_easy_perform() failed : " << curl_easy_strerror(res) << endl;
			curl_easy_cleanup(curl);

			GumboOutput *output = gumbo_parse(page.c_str());
			CollectFriends(output->root);
			gumbo_destroy_output(&kGumboDefaultOptions, output);
		}
	}

	return 0;
}
