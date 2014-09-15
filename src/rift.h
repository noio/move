#pragma once

#include "lights.h"
#include "ofxDropstuff.h"
#include "ofMain.h"
#include "ofxCv.h"


typedef struct PointMeta
{
    bool is_tear;
    float last_grown;
} PointMeta;

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

    void update(double delta_t, const ofxDS::FlowCam& flowcam_a, const ofxDS::FlowCam& flowcam_b);
    void updateOpen();
    void insertPoints(float insert_point_dist);
    void updateSize(const ofxDS::FlowCam& flowcam_a, const ofxDS::FlowCam& flowcam_b);

    void drawMask();
    void drawDebug();
    void drawLights(Lights lights);
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
    static bool grow_directional;
    static float grow_speed;
    static float shrink_speed;
    static float shrink_delay;

    bool changed = false;
    float area = 0.0;
    float fade = 0.001;
    float age = 0.0;
    float time_since_grow = 0.0;
    float do_open = true;

private:
    ofPolyline initial_line, points;
    vector<PointMeta> meta;

    double resample_timer;

    void resample();

};

