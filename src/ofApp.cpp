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
    rgb_there = new VideoFeedStatic();
    ((VideoFeedStatic*) rgb_there)->setup("stockholm.jpg");
    rgb_there->setAspectRatio(ofGetWidth(), ofGetHeight());
    //
    rgb_here = new VideoFeedWebcam();
    ((VideoFeedWebcam*) rgb_here)->setup(0, 1280, 720);
    rgb_here->setAspectRatio(ofGetWidth(), ofGetHeight());
    //
    //    flowcam_there.setup(capture_width, capture_height, ofGetWidth(), ofGetHeight(), 1, 1.0);
}

//--------------------------------------------------------------
void ofApp::update()
{
    delta_t = ofGetLastFrameTime();
    cv::Mat frame_here;
    ofLogVerbose("ofApp") << "update";
    if (rgb_here->getFrame(frame_here))
    {
        ofLogVerbose("ofApp") << "new frame <===";
        flowcam_here.update(frame_here, delta_t);
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
    
    float flow_width = flowcam_here.getFlowHigh().cols;
    if (flow_width > 0)
    {
        float scale_flow_to_game = ofGetWidth() / flow_width;
        ofPushMatrix();
        ofScale(scale_flow_to_game, scale_flow_to_game);
        vector<ofPolyline> contours = flowcam_here.getContoursHigh();
        for (int i = 0; i < contours.size(); i ++){
            ofBeginShape();
            vector<ofPoint>& vertices = contours[i].getVertices();
            for(int j = 0; j < vertices.size(); j++)
            {
                ofVertex(vertices[j]);
            }
            ofEndShape();
        }
        ofPopMatrix();
    }
//    ofSetColor(255, 255, 255, 255);
//    ofCircle(ofGetWidth() / 2, ofGetHeight() / 2, 40);
    //
    maskBeginContent();
    rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    maskEnd();
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
