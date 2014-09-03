#include "light.h"

float Light::ray_length = 0.2;
float Light::ray_rift_normal_bias = 10;

void Light::setup()
{
    position = ofPoint(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
    color = ofColor::fromHsb(ofRandom(255), 255, 255);
}

void Light::moveTo(const ofPoint& target)
{
    position += .1 * (target - position);
    velocity.set(0, 0);
}

void Light::update(double delta_t)
{
    velocity += ofPoint(ofRandomf(), ofRandomf()) * delta_t * 30;
    velocity += .001 * (ofPoint(ofGetWidth() * .5, ofGetHeight() * .5) - position);
    position += velocity * delta_t;
    position.x = ofClamp(position.x, 0, ofGetWidth());
    position.y = ofClamp(position.y, 0, ofGetHeight());
}

void Light::drawDebug()
{
    ofSetColor(color, 255);
    ofCircle(position, 10);
    ofSetColor(ofColor::white);
}