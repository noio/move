#pragma once

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

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void createRifts();

    ofPtr<VideoFeed> rgb_there;
    ofPtr<VideoFeed> rgb_here;
    FlowCam flowcam_here;
    FlowCam flowcam_there;
    ContourFinder contourfinder;

    vector<Rift> rifts;
    vector<Light> lights;

    float delta_t;

    int capture_width = 1280;
    int capture_height = 720;

    bool draw_debug = true;
    int max_rifts = 3;
    int new_rift_min_flow = 200;
};
