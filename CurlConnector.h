#ifndef CURLCONNECTOR_H
#define CURLCONNECTOR_H

#include <curl/curl.h>

#include <iostream>
#include <string>

using namespace std;

class CurlConnector {
    public : 
        CurlConnector() {};
        string getPage(string url);
    private :
    	CURL *curl;
    	CURLcode res;

        size_t curlToString(void *ptr, size_t size, size_t nmemb, void *data);
};

#endif /* CURLCONNECTOR_H */