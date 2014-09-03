#ifndef MOVE_RIFT_H_
#define MOVE_RIFT_H_

#include "light.h"
#include "flowcam.h"

#include "ofMain.h"
#include "ofxCv.h"

using namespace cv;

class Rift
{

public:
    Rift() {};
//    Rift(const Rift&) = delete;            // no copy
//    Rift& operator=(const Rift&) = delete; // no assign

    void setup();
    void setup(const ofPoint& center);
    void setup(const ofRectangle& bbox);
    void setup(ofPolyline initial);

    void update(double delta_t, const FlowCam& flowcam_a, const FlowCam& flowcam_b);
    void updateOpen();
    void insertPoints(float insert_point_dist);
    void updateSize(const FlowCam& flowcam_a, const FlowCam& flowcam_b);

    void drawMask();
    void drawDebug();
    void drawLights(const vector<Light>& lights);
    void drawOutline();
    void drawInnerLight();

    ofRectangle getBoundingBox() { return points.getBoundingBox(); };

    static float inner_light_strength;
    static float fade_max_area;
    static float fade_in_time;
    static float fade_out_time;
    static float open_time;
    static float resample_time;
    static float max_point_dist;
    static float grow_min_flow_squared;
    static float grow_speed;
    static float shrink_speed;
    static float shrink_delay;
    static float tear_heat;

    bool changed = false;
    float area = 0.0;
    float fade = 0.001;
    float age = 0.0;
    float time_since_grow = 0.0;
    float do_open = true;

private:
    ofPolyline initial_line, points;
    vector<float> heat;

    double resample_timer;

    void resample();

};


#endif /* defined(MOVE_RIFT_H_) */
