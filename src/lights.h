#pragma once

#include "rift.h"
#include "skeletonfeed.h"
#include "ofMain.h"

using namespace std;

typedef struct Light
{
    ofColor color;
    ofPoint position;
    ofPoint velocity;
    ofPoint target;
    float rotation;
} Light;

class Lights
{

public:
    Lights() {};

    void setup(int num_lights);
    void update(double delta_t, const vector<ofxDS::Skeleton>& skeletons);
    void retarget(const std::vector<Rift>& rifts);
    void draw();
    void drawDebug();

    vector<Light> lights;

    float ray_length = 0.2;
    float ray_rift_normal_bias = 10;
    
private:
    void orbit(double delta_t, Light& light);

};

