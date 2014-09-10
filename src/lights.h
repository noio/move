#ifndef MOVE_LIGHTS_H_
#define MOVE_LIGHTS_H_

#include "skeletonfeed.h"
#include "ofMain.h"

using namespace std;

typedef struct Light
{
    ofColor color;
    ofPoint position;
    ofPoint velocity;
    float rotation;
} Light;

class Lights
{

public:
    Lights() {};

    void setup(int num_lights);
    void update(double delta_t, const vector<ofxDS::Skeleton>& skeletons);
    void draw();
    void drawDebug();

    vector<Light> lights;

    float ray_length = 0.2;
    float ray_rift_normal_bias = 10;

};


#endif /* defined(MOVE_LIGHTS_H_) */
