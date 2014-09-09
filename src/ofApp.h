#pragma once

#include "skeletonfeed.h"
#include "rift.h"
#include "lights.h"

#include "ofxDropstuff.h"
#include "ofxCv.h"
#include "ofxJSON.h"
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

enum VideoSource
{
    VIDEO_SOURCE_PLACEHOLDER,
    VIDEO_SOURCE_WEBCAM0,
    VIDEO_SOURCE_WEBCAM1,
    VIDEO_SOURCE_SERVER_LOCAL,
    VIDEO_SOURCE_SERVER_REMOTE,
    VIDEO_SOURCE_CUSTOM_URL,
};

class ofApp : public ofBaseApp
{

public:
    void setup();
    void update();
    void draw();

    void loadConfig();
    void setupUI();
    ofxDS::VideoFeed* setupVideoFeed(VideoSource source);

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

    ofxJSONElement config;

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

    // Settings
    int window_x = 20, window_y = 100, window_width = 896, window_height = 288;
    int rgb_here_fps = 30, rgb_there_fps = 30;
    VideoSource rgb_here_source = VIDEO_SOURCE_WEBCAM0;
    VideoSource rgb_there_source = VIDEO_SOURCE_PLACEHOLDER;
    int rgb_here_flip = 1;
    int rgb_there_flip = 2;
    std::string source_custom_url = "http://192.168.1.14/color";
    bool use_skeletons = false;

    bool draw_debug = true;
    bool disable_local_rgb = false;
    DebugOverlay debug_overlay = DEBUGOVERLAY_NONE;

    float create_rifts_time = 2.0f;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
    ofColor rgb_here_multiply = ofColor(200);
    float flow_hist_darkness = 0.8;
};
