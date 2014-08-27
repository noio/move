#include "rift.h"
#include "utilities.h"

void Rift::setup()
{
    float x = ofRandom(.1, .9);
    points.addVertex(ofGetWidth() * (x - .1), ofGetHeight() * .5);
    points.addVertex(ofGetWidth() * (x + .1), ofGetHeight() * .5);
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
            if (heat[i] <= 1)
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
        ofMesh mesh;
        const Light& light = lights[li];
        ofPoint p1, p2, p3;
        int midx = 0;
        for(int pidx = 0; pidx <= (int) points.size() - 1; pidx++ )
        {
            int pwrap = pidx % points.size();
            ofPoint n1 = points[pwrap];
            ofPoint ray = (n1 - light.position) * light.ray_length;
            ofPoint n2 = n1 + ray * .5 + points.getNormalAtIndex(pwrap) * light.ray_rift_normal_bias;
            ofPoint n3 = n1 + ray;
            int alpha = 10 * 255 * 255 / n1.squareDistance(light.position);
            mesh.addVertex(n1);
            mesh.addColor(ofColor(light.color, alpha));
            int n1idx = midx;
            int p1idx = midx - 3;
            mesh.addVertex(n2);
            mesh.addColor(ofColor(light.color, alpha));
            int n2idx = midx + 1;
            int p2idx = midx - 2;
            mesh.addVertex(n3);
            mesh.addColor(ofColor(light.color, alpha));
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