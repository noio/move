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
    
    float max_dist_squared = 2000;
    float min_dist_squared = 100;
private:
    ofPolyline points;
    
};


#endif /* defined(MOVE_RIFT_H_) */
