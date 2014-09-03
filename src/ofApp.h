#pragma once

#include "skeletonfeed.h"
#include "videofeed.h"
#include "flowcam.h"
#include "rift.h"
#include "lights.h"

#include "ofxCv.h"
#include "ofMain.h"

#include <deque>

using namespace ofxCv;
using namespace cv;

enum DebugDrawFlow
{
    DEBUGDRAW_FLOW_NONE,
    DEBUGDRAW_FLOW_HERE,
    DEBUGDRAW_FLOW_THERE,
    DEBUGDRAW_FLOWHIST_HERE,
    DEBUGDRAW_FLOWHIST_THERE,
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
    ofPtr<VideoFeed> rgb_there;
    ofPtr<VideoFeed> rgb_here;
    FlowCam flowcam_here;
    FlowCam flowcam_there;
    Mat flow_hist_threshold, flow_here_hist, flow_there_hist;
    ContourFinder contourfinder;

    vector<Rift> rifts;
    Lights lights;

    float delta_t;
    double create_rifts_timer;

    bool draw_debug = true;
    bool disable_local_rgb = false;
    DebugDrawFlow debug_draw_flow_mode = DEBUGDRAW_FLOW_NONE;

    float create_rifts_time = 2.0f;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
    ofColor rgb_here_multiply = ofColor(200);
    float flow_hist_darkness = 0.8;
};
