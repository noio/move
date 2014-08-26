#include "rift.h"
#include "utilities.h"

void Rift::setup()
{
    points.addVertex(ofGetWidth() * .4, ofGetHeight() * .5);
    points.addVertex(ofGetWidth() * .6, ofGetHeight() * .5);
    points.close();
    heat.push_back(tear_heat);
    heat.push_back(tear_heat);
}

void Rift::update(double delta_t, const FlowCam& flowcam)
{
    float max_dist_squared = max_dist * max_dist;
    for (int i = 0; i < points.size(); i ++)
    {
        int j = (i + 1) % points.size();
        ofPoint a = points[i];
        ofPoint b = points[j];
        float d = ofDistSquared(a.x, a.y, b.x, b.y);
        if (heat[i] < tear_heat)
        {
            heat[i] *= heat_decay;
        }
        if (d > max_dist_squared)
        {
            points.insertVertex((a + b) * .5, i + 1);
            heat.insert(heat.begin() + i + 1, 1.0f);
        }
    }
    //
    const Mat flow_high = flowcam.getFlowHigh();
    float scale_game_to_flow = (float)flow_high.cols / ofGetWidth();
    //
    for (unsigned int i = 0; i < points.size(); i ++)
    {
        const ofPoint& cur = points[i];
        ofPoint p = cur * scale_game_to_flow;
        uchar flow = flow_high.at<uchar>(p.y, p.x);
        // I don't know why this would be 128. I thought binary cv::Mats were 0-255 only,
        // but the erosion/dilation yields intermediary values I suppose.
        if (flow > 128)
        {
            ofPoint normal = points.getNormalAtIndex(i);
            if (flowcam.getFlowAt(p.x, p.y).normalized().dot(normal) > 0)
            {
                heat[i] = MAX(heat[i], 2.0f);
                ofPoint moved = cur + normal * grow_speed * heat[i];
                if (!points.inside(moved))
                {
                    points[i] = moved;
                }
            }
        }
        else
        {
            if (heat[i] < 1)
            {
                ofPoint moved = cur - shrink_speed * points.getNormalAtIndex(i);
                if (points.inside(moved))
                {
                    points[i] = moved;
                }
            }
        }
    }
    resample_timer += delta_t;
    if (resample_timer > resample_time)
    {
        resample_timer = 0;
        resample();
    }
}

void Rift::resample()
{
    points = points.getResampledBySpacing(max_dist / 2);
    points = points.getSmoothed(2);
    heat.resize(points.size());
    std::fill(heat.begin(), heat.end(), 1.0f);
    for (int i = rand() % tear_frequency; i < heat.size(); i += tear_frequency)
    {
        heat[i] = tear_heat;
    }
}


void Rift::drawMask()
{
//    ofSetLineWidth(3.0f);
    ofBeginShape();
    for(int j = 0; j < points.size(); j++)
    {
        ofVertex(points[j]);
    }
    ofEndShape();
//    points.draw();
}

void Rift::drawLights(const vector<Light>& lights)
{
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    for (int li = 0; li < lights.size(); li ++)
    {
        const Light& light = lights[li];
        for(int pi = 0; pi < (int) points.size(); pi++ )
        {
            const ofPoint& cur = points[pi];
            int pj = (pi + 1) % points.size();
            const ofPoint& next = points[pj];
            ofPoint exc = cur + (cur - light.position) * light.ray_length;
            ofPoint exn = next + (next - light.position) * light.ray_length;
            int alpha = 10 * 255 * 255 / cur.squareDistance(light.position);
            ofSetLineWidth(0);
            ofSetColor(light.color, alpha);
            ofBeginShape();
            ofVertex(cur.x, cur.y);
            ofVertex(next.x, next.y);
            ofVertex(exn);
            ofVertex(exc);
            ofEndShape();
        }
    }
    ofDisableBlendMode();
    ofSetColor(255, 255, 255, 255);
}

void Rift::drawOutline()
{
    ofSetLineWidth(20);
    ofSetColor(0, 0, 0, 128);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    points.draw();
    ofSetColor(ofColor::white);
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