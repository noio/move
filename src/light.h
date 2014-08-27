#ifndef MOVE_LIGHT_H_
#define MOVE_LIGHT_H_

#include "ofMain.h"

class Light
{

public:
    Light() {};

    void setup();
    void update(double delta_t);

    void drawDebug();

    static float ray_length;
    static float ray_rift_normal_bias;

    ofColor color;
    ofPoint position;
    ofPoint velocity;
};


#endif /* defined(MOVE_LIGHT_H_) */
