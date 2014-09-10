#include "fileloader.h"

#include "Poco/Net/HTTPSession.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"

using namespace Poco::Net;
using namespace Poco;

/*
 * This is a copy of ofURLFileLoader.handleRequest that allows re-use of an
 * existing session.
 */
ofHttpResponse loadRequest(ofHttpRequest request, ofPtr<HTTPSession> session)
{
    try
    {
        URI uri(request.url);
        std::string path(uri.getPathAndQuery());
        if (path.empty()) path = "/";
        HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        HTTPResponse res;
        istream * rs;
        if(uri.getScheme() == "https")
        {
            ofLogError("loadRequest") << "Cannot load HTTPS.";
            return ofHttpResponse(request, -1, "HTTPS not supported");
        }
        else
        {
            HTTPClientSession * httpSession = new HTTPClientSession(uri.getHost(), uri.getPort());
            httpSession->setTimeout(Poco::Timespan(20, 0));
            httpSession->sendRequest(req);
            rs = &httpSession->receiveResponse(res);
            session = ofPtr<HTTPSession>(httpSession);
        }
        if(!request.saveTo)
        {
            return ofHttpResponse(request, *rs, res.getStatus(), res.getReason());
        }
        else
        {
            ofFile saveTo(request.name, ofFile::WriteOnly, true);
            char aux_buffer[1024];
            rs->read(aux_buffer, 1024);
            std::streamsize n = rs->gcount();
            while (n > 0)
            {
                // we resize to size+1 initialized to 0 to have a 0 at the end for strings
                saveTo.write(aux_buffer, n);
                if (rs->good())
                {
                    rs->read(aux_buffer, 1024);
                    n = rs->gcount();
                }
                else n = 0;
            }
            return ofHttpResponse(request, res.getStatus(), res.getReason());
        }
    }
    catch (const Poco::Exception& exc)
    {
        ofLogError("ofURLFileLoader") << "(): " + exc.displayText();
        return ofHttpResponse(request, -1, exc.displayText());
    }
    catch (...)
    {
        return ofHttpResponse(request, -1, "ofURLFileLoader: fatal error, couldn't catch Exception");
    }
    return ofHttpResponse(request, -1, "ofURLFileLoader: fatal error, couldn't catch Exception");
}

ofHttpResponse loadURL(string url, ofPtr<HTTPSession> session)
{
    ofHttpRequest request(url, url);
    return loadRequest(request, session);
}
