#include "CurlConnector.h"

size_t curlToString(void *ptr, size_t size, size_t nmemb, void *data) {
	string *str = (string *)data;
	char *sptr = (char *) ptr;

	for(size_t i = 0; i < size*nmemb; i++) {
		(*str) += sptr[i];
	}

	return size*nmemb;
}

string CurlConnector::getPage(string url) {
    string page = "";
    
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlToString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			cout << "Error : curl_easy_perform() failed : " << curl_easy_strerror(res) << endl;
		curl_easy_cleanup(curl);
	}
	
	return page;
}
