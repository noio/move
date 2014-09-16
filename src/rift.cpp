#include "rift.h"
#include "lights.h"
#include "utilities.h"

using namespace cv;
using namespace ofxDS;

float Rift::inner_light_strength = 1000;
float Rift::fade_max_area = 300;
float Rift::open_time = 1.0f;
float Rift::fade_in_time = 1.0f;
float Rift::fade_out_time = 10.0f;
float Rift::resample_time = 20.0f;
float Rift::max_point_dist = 40;
float Rift::grow_speed = 0.4f;
float Rift::grow_min_flow_squared = 1600;
bool Rift::grow_directional = false;
float Rift::shrink_speed = 0.1f;
float Rift::shrink_delay = 3.0f;

void Rift::setup()
{
    ofPolyline initial;
    float x = ofRandom(.1, .9);
    initial.addVertex(ofGetWidth() * (x - .1), ofGetHeight() * .5);
    initial.addVertex(ofGetWidth() * (x + .1), ofGetHeight() * .5);
    initial.close();
    setup(initial);
}


void Rift::setup(const ofRectangle& bbox)
{
    const int num_points = ceil(bbox.height / (0.5f * max_point_dist));
    const float step = 1.0f / (num_points - 1);
    const float x_mid = bbox.getCenter().x;
    ofPolyline initial;
    for (int i = 0; i < num_points; i ++)
    {
        const float r = i * step;
        const float scale = r * (1 - r) * 4;
        float x = x_mid + ofRandomf() * bbox.width * .2 * scale;
        float y = bbox.y + r * bbox.height;
        initial.addVertex(ofPoint(x, y));
    }
    setup(initial);
}

void Rift::setup(ofPolyline initial)
{
    if (!initial.isClosed())
    {
        do_open = true;
        initial_line = initial;
        points.clear();
        points.addVertex(initial_line.getPointAtPercent(0.49));
        points.addVertex(initial_line.getPointAtPercent(0.51));
        points.close();
    }
    else
    {
        do_open = false;
        points = initial;
    }
    meta.clear();
    float current_time = ofGetElapsedTimef();
    for (int i = 0; i < points.size(); i ++)
    {
        meta.push_back( (PointMeta) {false, current_time} );
    }
    resample();
}

void Rift::update(double delta_t, const FlowCam& flowcam_a, const FlowCam& flowcam_b)
{
    int i = 0;
    ofLogVerbose("Rift") << i++;
    changed = false;

    if (do_open && age < open_time)
    {
        updateOpen();
        insertPoints(max_point_dist * .5);
    }
    else
    {
        updateSize(flowcam_a, flowcam_b);
        if (time_since_grow == 0)
        {
            insertPoints(max_point_dist);
        }
    }
    ofLogVerbose("Rift") << i++;
    if (changed)
    {
        area = MAX(0.1, points.getArea());
    }
    if (area < fade_max_area && age > fade_in_time + open_time)
    {
        fade = MAX(0.0, fade - delta_t / fade_out_time);
    }
    else
    {
        fade = MIN(1.0, fade + delta_t / fade_in_time);
    }
    //
    age += delta_t;
    time_since_grow += delta_t;
    resample_timer += delta_t;
    ofLogVerbose("Rift") << i++;
    if (resample_timer > resample_time)
    {
        resample_timer = 0;
        resample();
    }
    if (changed) {
        tessellator.tessellateToMesh(points, OF_POLY_WINDING_POSITIVE, shape);
    }
}

void Rift::updateOpen()
{
    ofLogVerbose("Rift") << "updateOpen";
    const float f = 0.5 + 0.5 * age / open_time;
    ofPoint a = initial_line.getPointAtPercent(f);
    ofPoint b = initial_line.getPointAtPercent(1 - f);
    unsigned int nearestIndex;
    points.getClosestPoint(a, &nearestIndex);
    points[nearestIndex] = a;
    points.getClosestPoint(b, &nearestIndex);
    points[nearestIndex] = b;
    ofLogVerbose("Rift") << "updateOpen return";
}

void Rift::insertPoints(float insert_point_dist)
{
    ofLogVerbose("Rift") << "insertPoints";
    const float max_dist_squared = insert_point_dist * insert_point_dist;
    for (int i = 0; i < points.size(); i ++)
    {
        int j = (i + 1) % points.size();
        const ofPoint a = points[i];
        const ofPoint b = points[j];
        float d = ofDistSquared(a.x, a.y, b.x, b.y);
        if (d > max_dist_squared)
        {
            points.insertVertex((a + b) * .5, i + 1);
            meta.insert(meta.begin() + i + 1, (PointMeta) {false, 1.0f});
            changed = true;
        }
    }
    ofLogVerbose("Rift") << "insertPoints end";
}

void Rift::updateSize(const FlowCam& flowcam_a, const FlowCam& flowcam_b)
{
    ofLogVerbose("Rift") << "updateSize " << ofGetWidth() << "x" << ofGetHeight();
    ofLogVerbose("Rift") << "a_data: " << flowcam_a.hasData() << "  b_data: " << flowcam_b.hasData();
    const ofPoint screen = ofPoint(ofGetWidth(), ofGetHeight());
    float current_time = ofGetElapsedTimef();
    for (unsigned int i = 0; i < points.size(); i ++)
    {
        const ofPoint& cur = points[i];
        const ofPoint p = cur / screen;
        const ofPoint normal = points.getNormalAtIndex(i);
        ofVec2f flow_a;
        if (flowcam_a.hasData()) {
            flow_a =  flowcam_a.getFlowAtUnitPos(p);
        }
        ofVec2f flow_b;
        if (flowcam_b.hasData()) {
            flow_b = flowcam_b.getFlowAtUnitPos(p);
        }
        if ((flow_a.lengthSquared() > grow_min_flow_squared && (!grow_directional || flow_a.dot(normal) > 0)) ||
            (flow_b.lengthSquared() > grow_min_flow_squared && (!grow_directional || flow_b.dot(normal) > 0)))
        {
            float gs = grow_speed;
            ofLogVerbose("Rift") << "grow " << i << " by " << gs;
            if (meta[i].is_tear)
            {
                gs *= 4;
            }
            ofPoint moved = cur + normal * gs;
            moved = ofPoint(ofClamp(moved.x, 0, ofGetWidth()), ofClamp(moved.y, 0, ofGetHeight()));
            if (!points.inside(moved))
            {
                points[i] = moved;
                meta[i].last_grown = current_time;
                changed = true;
                time_since_grow = 0;
            }
        }
        else if (current_time - meta[i].last_grown >= shrink_delay)
        {
            ofPoint moved = cur - shrink_speed * points.getNormalAtIndex(i);
            moved = ofPoint(ofClamp(moved.x, 0, ofGetWidth()), ofClamp(moved.y, 0, ofGetHeight()));
            if (points.inside(moved))
            {
                points[i] = moved;
                meta[i].is_tear = false;
                changed = true;
            }
        }
    }
    ofLogVerbose("Rift") << "updateSize returns";
}

void Rift::resample()
{
    if (points.size() > 5)
    {
        points = points.getResampledBySpacing(max_point_dist / 2);
        points = points.getSmoothed(2);
        meta.resize(points.size());
        if (points.size() > 1)
        {
            unsigned int nearestIndex;
            points.getClosestPoint(ofPoint(0, ofGetHeight() * .5), &nearestIndex);
            meta[nearestIndex].is_tear = true;
            points.getClosestPoint(ofPoint(ofGetWidth(), ofGetHeight() * .5), &nearestIndex);
            meta[nearestIndex].is_tear = true;
        }
        changed = true;
    }
}


void Rift::drawMask()
{
    ofSetColor(255, 255);
    shape.draw();
}

void Rift::drawLights(Lights* lights)
{
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    for (int li = 0; li < lights->lights.size(); li ++)
    {
        ofMesh mesh;
        const Light& light = lights->lights[li];
        int midx = 0;
        for(int pidx = 0; pidx <= (int) points.size(); pidx++ )
        {
            int pwrap = pidx % points.size();
            const ofPoint& n1 = points[pwrap];
            ofPoint ray = (n1 - light.position) * lights->ray_length;
            ofPoint n2 = n1 + ray * .5 + points.getNormalAtIndex(pwrap) * lights->ray_rift_normal_bias;
            ofPoint n3 = n1 + ray;
            int alpha = MIN(255, 10 * 255 * 255 / n1.squareDistance(light.position)) * fade;
            //alpha *= MAX(0, points.getNormalAtIndex(pwrap).dot(ray.normalized()));
            alpha *= fade;
            mesh.addVertex(n1);
            mesh.addColor(ofColor(light.color, alpha));
            int n1idx = midx;
            int p1idx = midx - 3;
            mesh.addVertex(n2);
            mesh.addColor(ofColor(light.color, 0));
            int n2idx = midx + 1;
            int p2idx = midx - 2;
            mesh.addVertex(n3);
            mesh.addColor(ofColor(light.color, 0));
            int n3idx = midx + 2;
            int p3idx = midx - 1;
            midx += 3;
            if (pidx > 0)
            {
                // Short ray
                mesh.addTriangle(p1idx, n1idx, n2idx);
                mesh.addTriangle(p1idx, n2idx, p2idx);
                // Long ray
                mesh.addTriangle(p1idx, n1idx, n3idx);
                mesh.addTriangle(p1idx, n3idx, p3idx);
            }
        }
        mesh.draw();
    }
    ofDisableBlendMode();
    ofSetColor(255, 255, 255, 255);
}

void Rift::drawOutline()
{
    ofSetLineWidth(20);
    ofSetColor(0, 0, 0, 128 * fade);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    points.draw();
    ofSetColor(ofColor::white);
    ofDisableBlendMode();
}

void Rift::drawInnerLight()
{
    ofEnableAlphaBlending();
    int alpha = MIN(255 * inner_light_strength / area, 255);
    ofSetColor(fade * 255, alpha);
    ofFill();
    shape.draw();
    ofSetColor(ofColor::white);
    ofDisableAlphaBlending();
    // INNER GLOW
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
//    for (int li = 0; li < lights.lights.size(); li ++)
//    {
//        ofMesh mesh;
//        const Light& light = lights.lights[li];
//        int midx = 0;
//        for(int pidx = 0; pidx <= (int) points.size(); pidx++ )
//        {
//            int pwrap = pidx % points.size();
//            const ofPoint& n1 = points[pwrap];
////            ofPoint ray = (n1 - light.position) * lights.ray_length;
//            ofPoint n2 = n1 - 20 * points.getNormalAtIndex(pwrap);
//            int alpha = MIN(255, 10 * 255 * 255 / n1.squareDistance(light.position)) * fade;
//            alpha *= fade;
//            mesh.addVertex(n1);
//            mesh.addColor(ofColor(light.color, alpha));
//            int n1idx = midx;
//            int p1idx = midx - 2;
//            mesh.addVertex(n2);
//            mesh.addColor(ofColor(light.color, 0));
//            int n2idx = midx + 1;
//            int p2idx = midx - 1;
//            midx += 2;
//            if (pidx > 0)
//            {
//                // Short ray
//                mesh.addTriangle(p1idx, n1idx, n2idx);
//                mesh.addTriangle(p1idx, n2idx, p2idx);
//            }
//        }
//        mesh.draw();
//    }
//    ofDisableBlendMode();
//    ofSetColor(255, 255, 255, 255);

}

void Rift::drawDebug()
{
    ofSetLineWidth(2.0f);
    for(int i = 0; i < (int) points.size(); i++ )
    {
        const ofPoint& cur = points[i];
        int j = (i + 1) % points.size();
        const ofPoint& next = points[j];
        ofLine(cur, next);
        if (meta[i].is_tear)
        {
            ofSetColor(255, 0, 255);
        }
        ofLine(cur, cur + points.getNormalAtIndex(i) * 10);
        ofSetColor(255, 255, 255);
    }
}
