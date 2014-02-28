#include <stdio.h>
#include <curl/curl.h>
#include <gumbo.h>

#include <iostream>
#include <string>

using namespace std;

size_t curl_to_string(void *ptr, size_t size, size_t nmemb, void *data) {
	string *str = (string *)data;
	char *sptr = (char *) ptr;

	for(size_t i = 0; i < size*nmemb; i++) {
		(*str) += sptr[i];
	}

	return size*nmemb;
}

int main(int argc, char** argv) {
	if(argc != 2) {
		cout << "Usage : Crawler 'Keyword'" << endl << endl;
		return -1;
	}

	string keyword = argv[1];
	string page;

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {
		string url = "http://steamcommunity.com/id/" + keyword;
		page = "";

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			cout << "Error : curl_easy_perform() failed : " << curl_easy_strerror(res) << endl;
		curl_easy_cleanup(curl);

		GumboOutput *root = gumbo_parse(page.c_str());

		gumbo_destroy_output(&kGumboDefaultOptions, root);
	}

	return 0;
}
