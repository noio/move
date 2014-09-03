#include "rift.h"
#include "utilities.h"

float Rift::inner_light_strength = 1000;
float Rift::fade_max_area = 300;
float Rift::open_time = 1.0f;
float Rift::fade_in_time = 1.0f;
float Rift::fade_out_time = 10.0f;
float Rift::resample_time = 20.0f;
float Rift::max_point_dist = 40;
float Rift::grow_speed = 0.4f;
float Rift::grow_min_flow_squared = 1600;
float Rift::shrink_speed = 0.1f;
float Rift::shrink_delay = 3.0f;
float Rift::tear_heat = 8.0f;



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
    heat.clear();
    for (int i = 0; i < points.size(); i ++)
    {
        heat.push_back(1.0f);
    }
    resample();
}

void Rift::update(double delta_t, const FlowCam& flowcam_a, const FlowCam& flowcam_b)
{
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
    if (resample_timer > resample_time)
    {
        resample_timer = 0;
        resample();
    }
}

void Rift::updateOpen()
{
    const float f = 0.5 + 0.5 * age / open_time;
    ofPoint a = initial_line.getPointAtPercent(f);
    ofPoint b = initial_line.getPointAtPercent(1 - f);
    unsigned int nearestIndex;
    points.getClosestPoint(a, &nearestIndex);
    points[nearestIndex] = a;
    points.getClosestPoint(b, &nearestIndex);
    points[nearestIndex] = b;
}

void Rift::insertPoints(float insert_point_dist)
{
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
            heat.insert(heat.begin() + i + 1, 1.0f);
            changed = true;
        }
    }
}

void Rift::updateSize(const FlowCam& flowcam_a, const FlowCam& flowcam_b)
{
    const ofPoint screen = ofPoint(ofGetWidth(), ofGetHeight());
    for (unsigned int i = 0; i < points.size(); i ++)
    {
        const ofPoint& cur = points[i];
        const ofPoint p = cur / screen;
        const ofPoint normal = points.getNormalAtIndex(i);
        const ofVec2f flow_a = flowcam_a.getFlowAtUnitPos(p.x, p.y);
        const ofVec2f flow_b = flowcam_b.getFlowAtUnitPos(p.x, p.y);
        if ((flow_a.lengthSquared() > grow_min_flow_squared && flow_a.dot(normal) > 0) ||
                (flow_b.lengthSquared() > grow_min_flow_squared && flow_b.dot(normal) > 0))
        {
            heat[i] = MAX(heat[i], 2.0f);
            ofPoint moved = cur + normal * grow_speed * heat[i];
            moved = ofPoint(ofClamp(moved.x, 0, ofGetWidth()), ofClamp(moved.y, 0, ofGetHeight()));
            if (!points.inside(moved))
            {
                points[i] = moved;
                changed = true;
                time_since_grow = 0;
            }
        }
        else if (time_since_grow >= shrink_delay)
        {
            if (heat[i] <= 1)
            {
                ofPoint moved = cur - shrink_speed * points.getNormalAtIndex(i);
                moved = ofPoint(ofClamp(moved.x, 0, ofGetWidth()), ofClamp(moved.y, 0, ofGetHeight()));
                if (points.inside(moved))
                {
                    points[i] = moved;
                    changed = true;
                }
            }
        }
    }
}

void Rift::resample()
{
    if (points.size() > 5)
    {
        points = points.getResampledBySpacing(max_point_dist / 2);
        points = points.getSmoothed(2);
        heat.resize(points.size());
        std::fill(heat.begin(), heat.end(), 1.0f);
        if (points.size() > 1)
        {
            unsigned int nearestIndex;
            points.getClosestPoint(ofPoint(0, ofGetHeight() * .5), &nearestIndex);
            heat[nearestIndex] = tear_heat;
            points.getClosestPoint(ofPoint(ofGetWidth(), ofGetHeight() * .5), &nearestIndex);
            heat[nearestIndex] = tear_heat;
        }
    }
}


void Rift::drawMask()
{
    ofSetColor(255, 255);
    ofBeginShape();
    for(int j = 0; j < points.size(); j++)
    {
        ofVertex(points[j]);
    }
    ofEndShape();
}

void Rift::drawLights(Lights lights)
{
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    for (int li = 0; li < lights.lights.size(); li ++)
    {
        ofMesh mesh;
        const Light& light = lights.lights[li];
        int midx = 0;
        for(int pidx = 0; pidx <= (int) points.size(); pidx++ )
        {
            int pwrap = pidx % points.size();
            const ofPoint& n1 = points[pwrap];
            ofPoint ray = (n1 - light.position) * lights.ray_length;
            ofPoint n2 = n1 + ray * .5 + points.getNormalAtIndex(pwrap) * lights.ray_rift_normal_bias;
            ofPoint n3 = n1 + ray;
            int alpha = MIN(255, 10 * 255 * 255 / n1.squareDistance(light.position)) * fade;
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
    ofBeginShape();
    for(int j = 0; j < points.size(); j++)
    {
        ofVertex(points[j]);
    }
    ofEndShape();
    ofSetColor(ofColor::white);
    ofDisableAlphaBlending();
}

void Rift::drawDebug()
{
    ofSetLineWidth(2.0f);
    ofSetColor(255, 0, 255);
    for(int i = 0; i < (int) points.size(); i++ )
    {
        const ofPoint& cur = points[i];
        int j = (i + 1) % points.size();
        const ofPoint& next = points[j];
        ofLine(cur, next);
        ofLine(cur, cur + points.getNormalAtIndex(i) * 10 * heat[i]);
        ofSetColor(255, 255, 255);
    }
}