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
};

#endif /* CURLCONNECTOR_H */
