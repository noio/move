#pragma once

#include "ofMain.h"
#include "Poco/Net/HTTPSession.h"

class FileLoaderSession
{
public:
    ofHttpResponse loadRequest(ofHttpRequest request);
    ofHttpResponse loadURL(string url);
private:

    bool session_connected;
    ofPtr<Poco::Net::HTTPSession> session;
};

