#pragma once

#include "skeletonfeed.h"
#include "lights.h"
#include "rift.h"

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
    DEBUGOVERLAY_DEPTH
};

enum VideoSource
{
    VID_PLACEHOLDER,
    VID_WEBCAM0,
    VID_WEBCAM1,
    VID_LOCAL_FEED,
    VID_REMOTE_FEED,
    VID_REMOTE_SERVER,
    VID_CUSTOM_URL
};

class ofApp : public ofBaseApp
{

public:
    void setup();
    void update();
    void draw();

    void loadConfig();
    void setupUI();
    ofxDS::VideoFeed* setupVideoFeed(VideoSource source, std::string& description);

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

    std::string configpath;
    ofxJSONElement config;

    ofPtr<ofxDS::SkeletonFeed> skeletonfeed;
    ofPtr<ofxDS::VideoFeed> rgb_there;
    ofPtr<ofxDS::VideoFeed> rgb_here;
    ofPtr<ofxDS::VideoFeed16Bit> depth;
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
    bool use_jpg_feed = false;
    VideoSource rgb_here_source = VID_WEBCAM0;
    VideoSource rgb_there_source = VID_PLACEHOLDER;
    std::string rgb_here_source_string, rgb_there_source_string;
    int rgb_here_flip = 1;
    int rgb_there_flip = 2;
    std::string source_custom_url = "http://192.168.1.14/color";
    bool use_skeletons = false;

    bool remoteui_draw_notif = false;
    bool draw_debug = true;
    bool disable_local_rgb = false;
    DebugOverlay debug_overlay = DEBUGOVERLAY_NONE;

    bool trigger_remove_all = false;

    float create_rifts_time = 2.0f;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
    ofColor rgb_here_multiply = ofColor(200);
    float flow_hist_darkness = 0.8;
};
