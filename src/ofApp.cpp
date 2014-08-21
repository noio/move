#include "ofApp.h"
#include "ofxCv.h"

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
    rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    rgb_here->draw(0, 0, ofGetWidth(), ofGetHeight());
    flowcam_here.drawDebug();
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
