#ifndef MOVE_RIFT_H_
#define MOVE_RIFT_H_

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
    void update(double delta_t, const FlowCam& flowcam);
    void draw();
    void drawDebug();
  
    double resample_time = 5.0;
    float max_dist = 40;
    float grow_speed = 0.4;
    int tear_frequency = 10;
    float tear_heat = 5.0f;
    
private:
    ofPolyline points;
    vector<float> heat;

    
    double resample_timer;
    
    void resample();
    
};


#endif /* defined(MOVE_RIFT_H_) */
