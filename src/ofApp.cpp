#include "ofApp.h"
#include "ofxCv.h"
#include "ofxRemoteUIServer.h"

void maskBeginAlpha()
{
    ofFill();
    glColorMask(false, false, false, true);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1.0f);
    glEnable(GL_BLEND);
}

void maskBeginContent()
{
    glColorMask(true, true, true, true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

void maskEnd()
{
    glDisable(GL_BLEND);
}


//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    flowcam_here.setup(160);
    //
    VideoFeedImageUrl* rgb_there_p = new VideoFeedImageUrl();
//    rgb_there_p->setup("stockholm.jpg");
    rgb_there_p->setup("http://192.168.1.34:1338/color");
    rgb_there_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_there = ofPtr<VideoFeed>(rgb_there_p);
    //
    VideoFeedWebcam* rgb_here_p = new VideoFeedWebcam();
    rgb_here_p->setup(1, 1280, 720);
    rgb_here_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_here = ofPtr<VideoFeed>(rgb_here_p);
    //
    Rift r;
    r.setup();
    rifts.push_back(r);
    for (int i = 0; i < 5; i ++)
    {
        Light l;
        l.setup();
        lights.push_back(l);
    }
    // SETUP UI
    OFX_REMOTEUI_SERVER_SETUP(); //start server
    //Expose x and y vars to the server, providing a valid slider range
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Light::ray_length, 0, 2.0f);
    //load values from XML, as they were last saved (if they were)
    OFX_REMOTEUI_SERVER_LOAD_FROM_XML();
}

//--------------------------------------------------------------
void ofApp::update()
{
    delta_t = ofGetLastFrameTime();
    cv::Mat frame_here;
    if (rgb_here->getFrame(frame_here))
    {
        flowcam_here.update(frame_here, delta_t);
    }
    if (flowcam_here.hasData())
    {
        for (int i = 0; i < rifts.size(); i ++)
        {
            rifts[i].update(delta_t, flowcam_here);
        }
    }
    for (int i = 0; i < lights.size(); i++)
    {
        lights[i].update(delta_t);
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    ofClear(0);
//    glColorMask(true, true, true, false);
    ofSetColor(200);
    rgb_here->draw(0, 0, ofGetWidth(), ofGetHeight());
    //
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawOutline();
    }
    //
    maskBeginAlpha();
//    float flow_width = flowcam_here.getFlowHigh().cols;
//    if (flow_width > 0)
//    {
//        float scale_flow_to_game = ofGetWidth() / flow_width;
//        ofPushMatrix();
//        ofScale(scale_flow_to_game, scale_flow_to_game);
//        vector<ofPolyline> contours = flowcam_here.getContoursHigh();
//        for (int i = 0; i < contours.size(); i ++)
//        {
//            ofBeginShape();
//            vector<ofPoint>& vertices = contours[i].getVertices();
//            for(int j = 0; j < vertices.size(); j++)
//            {
//                ofVertex(vertices[j]);
//            }
//            ofEndShape();
//        }
//        ofPopMatrix();
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawMask();
    }
//    }
    //
    maskBeginContent();
    rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    maskEnd();
    //
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawLights(lights);
    }
    //
    if (draw_debug)
    {
        for (int i = 0; i < rifts.size(); i ++)
        {
            rifts[i].drawDebug();
        }
        for (int i = 0; i < lights.size(); i ++)
        {
            lights[i].drawDebug();
        }
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(), 0) + "fps (r11)", ofPoint(3, 13));
        ofDrawBitmapStringHighlight("[d]ebug view", ofPoint(3, 33));
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch (key)
    {
    case 'd':
        draw_debug = !draw_debug;
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
