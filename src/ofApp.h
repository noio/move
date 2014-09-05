#pragma once

#include "skeletonfeed.h"
#include "rift.h"
#include "lights.h"

#include "ofxDropstuff.h"
#include "ofxCv.h"
#include "ofMain.h"

#include <deque>

using namespace ofxCv;
using namespace cv;

enum DebugOverlay
{
    DEBUGOVERLAY_NONE,
    DEBUGOVERLAY_FLOW_HERE,
    DEBUGOVERLAY_FLOW_THERE,
    DEBUGOVERLAY_FLOWHIST_HERE,
    DEBUGOVERLAY_FLOWHIST_THERE,
};

class ofApp : public ofBaseApp
{

public:
    void setup();
    void update();
    void draw();

    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void updateFlowHist();
    void createRifts();

    ofPtr<SkeletonFeed> skeletonfeed;
    ofPtr<ofxDS::VideoFeed> rgb_there;
    ofPtr<ofxDS::VideoFeed> rgb_here;
    ofxDS::FlowCam flowcam_here;
    ofxDS::FlowCam flowcam_there;
    Mat flow_hist_threshold, flow_here_hist, flow_there_hist;
    
    ContourFinder contourfinder;

    vector<Rift> rifts;
    Lights lights;

    float delta_t;
    double create_rifts_timer;

    bool draw_debug = true;
    bool disable_local_rgb = false;
    DebugOverlay debug_overlay = DEBUGOVERLAY_NONE;

    float create_rifts_time = 2.0f;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
    ofColor rgb_here_multiply = ofColor(200);
    float flow_hist_darkness = 0.8;
};
