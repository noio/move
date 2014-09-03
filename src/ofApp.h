#pragma once

#include "skeletonfeed.h"
#include "videofeed.h"
#include "flowcam.h"
#include "maskedimage.h"
#include "rift.h"
#include "light.h"

#include "ofxCv.h"
#include "ofMain.h"

#include <deque>

using namespace ofxCv;
using namespace cv;

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

    ofPtr<SkeletonFeed> skeletons;
    ofPtr<VideoFeed> rgb_there;
    ofPtr<VideoFeed> rgb_here;
    FlowCam flowcam_here;
    FlowCam flowcam_there;
    Mat flow_hist_threshold, flow_here_hist, flow_there_hist;
    ContourFinder contourfinder;

    vector<Rift> rifts;
    vector<Light> lights;

    float delta_t;
    double create_rifts_timer;

    int capture_width = 1280;
    int capture_height = 720;

    bool draw_debug = true;
    float create_rifts_time = 2.0f;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
    ofColor rgb_here_multiply = ofColor(200);
    float flow_hist_darkness = 0.8;
};
