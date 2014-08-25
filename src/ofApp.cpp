#include "ofApp.h"
#include "ofxCv.h"

void maskBeginAlpha()
{
    glEnable(GL_BLEND);
    glColorMask(false, false, false, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1.0f);
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
    VideoFeedStatic* rgb_there_p = new VideoFeedStatic();
    rgb_there_p->setup("stockholm.jpg");
    rgb_there_p->setAspectRatio(ofGetWidth(), ofGetHeight());
//    ((VideoFeedImageUrl*) rgb_there)->setup("http://192.168.1.18:1338/color");
    rgb_there = ofPtr<VideoFeed>(rgb_there_p);
    //
    VideoFeedWebcam* rgb_here_p = new VideoFeedWebcam();
    rgb_here_p->setup(0, 1280, 720);
    rgb_here_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_here = ofPtr<VideoFeed>(rgb_here_p);
    //
    Rift r;
    r.setup();
    rifts.push_back(r);
}

//--------------------------------------------------------------
void ofApp::update()
{
    delta_t = ofGetLastFrameTime();
    cv::Mat frame_here;
    if (rgb_here->getFrame(frame_here))
    {
        flowcam_here.update(frame_here, delta_t);
        for (int i = 0; i < rifts.size(); i ++) {
            rifts[i].update(delta_t, flowcam_here);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofClear(0);
    glColorMask(true, true, true, false);
    rgb_here->draw(0, 0, ofGetWidth(), ofGetHeight());
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
    for (int i = 0; i < rifts.size(); i ++) {
        rifts[i].draw();
    }
//    }
    //
    maskBeginContent();
    rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    maskEnd();
    //
    for (int i = 0; i < rifts.size(); i ++) {
        rifts[i].drawDebug();
    }
    //
    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(),0) + "fps (r11)", ofPoint(3, 13));
//    ofDrawBitmapStringHighlight("[d]ebug view \n[g]ui \n[h]elp \n[ ] crash \n[i] sound on \n[o] sound off \n[r]eset", ofPoint(3,33));
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
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
