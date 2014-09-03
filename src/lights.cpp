#include "lights.h"

void Lights::setup(int num_lights)
{
    for (int i = 0; i < num_lights; i ++)
    {
        Light l;
        l.position = ofPoint(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        l.color = ofColor::fromHsb(ofRandom(255), 255, 255);
        lights.push_back(l);
    }
}

void Lights::update(double delta_t, const vector<Skeleton>& skeletons)
{
    for (int i = 0; i < lights.size(); i ++)
    {
        Light& light = lights[i];
        light.velocity += ofPoint(ofRandomf(), ofRandomf()) * delta_t * 30;
        light.velocity += .001 * (ofPoint(ofGetWidth() * .5, ofGetHeight() * .5) - light.position);
        light.position += light.velocity * delta_t;
        light.position.x = ofClamp(light.position.x, 0, ofGetWidth());
        light.position.y = ofClamp(light.position.y, 0, ofGetHeight());
    }
}

void Lights::drawDebug()
{
    for (int i = 0; i < lights.size(); i ++){
        Light& light = lights[i];
        ofSetColor(light.color, 255);
        ofCircle(light.position, 10);
        ofSetColor(ofColor::white);
    }
}