#pragma once

#include "ofMain.h"
#include "Poco/Net/HTTPClientSession.h"

class FileLoaderSession
{
public:
    void setup(std::string uri);
    void setup(std::string host, unsigned short port);
    
    ofHttpResponse loadRequest(ofHttpRequest request);
    ofHttpResponse loadURL(string url);

private:
    void checkSession();
    
    std::string host;
    unsigned short port;
    bool session_connected;
    ofPtr<Poco::Net::HTTPClientSession> session;
};

