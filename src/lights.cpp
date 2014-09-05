#include "lights.h"

void meshAddBeam(ofMesh& mesh, int center_idx, float length, float angle, float width)
{
    const ofPoint& center = mesh.getVertex(center_idx);
    ofColor outer_color = ofColor(mesh.getColor(center_idx), 0);
    const ofPoint l = center + ofPoint(cos(angle - width) * length, sin(angle - width) * length);
    const ofPoint r = center + ofPoint(cos(angle + width) * length, sin(angle + width) * length);
    mesh.addVertex(l);
    mesh.addColor(outer_color);
    mesh.addVertex(r);
    mesh.addColor(outer_color);
    mesh.addTriangle(center_idx, mesh.getNumVertices() - 2, mesh.getNumVertices() - 1);
}

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
    if (skeletons.size())
    {
        for (int i = 0; i < lights.size(); i ++)
        {
            Light& light = lights[i];
            light.velocity.set(0);
            light.position += .1 * (skeletons[i % skeletons.size()].head - light.position);
            light.rotation += .1;
        }
    }
    else
    {
        for (int i = 0; i < lights.size(); i ++)
        {
            Light& light = lights[i];
            light.velocity += ofPoint(ofRandomf(), ofRandomf()) * delta_t * 30;
            light.velocity += .001 * (ofPoint(ofGetWidth() * .5, ofGetHeight() * .5) - light.position);
            light.position += light.velocity * delta_t;
            light.position.x = ofClamp(light.position.x, 0, ofGetWidth());
            light.position.y = ofClamp(light.position.y, 0, ofGetHeight());
            light.rotation += .01;
        }
    }
}

void Lights::draw()
{
    ofMesh mesh;
    for (int i = 0; i < lights.size(); i ++)
    {
        const Light& light = lights[i];
        int mid = mesh.getNumVertices();
        mesh.addVertex(light.position);
        mesh.addColor(light.color);
        meshAddBeam(mesh, mid, ray_length * ofGetHeight(), light.rotation, .5);
        meshAddBeam(mesh, mid, ray_length * ofGetHeight(), light.rotation + PI, .5);
    }
    mesh.draw();
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