#include "ofApp.h"
#include "ofxCv.h"

//--------------------------------------------------------------
void ofApp::setup()
{
//    flowcam_here.setup(capture_width, capture_height, ofGetWidth(), ofGetHeight(), 0, 1.0);
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
    flowcam_here.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    flowcam_here.draw(0, 0, ofGetWidth(), ofGetHeight());
    cv::Mat frame;
    rgb_there->getFrame(frame);
    ofxCv::drawMat(frame, 0, 0, ofGetWidth(), ofGetHeight());
    rgb_here->getFrame(frame);
    ofxCv::drawMat(frame, 0, 0, ofGetWidth(), ofGetHeight());
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
