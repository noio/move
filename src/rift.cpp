#include "rift.h"
#include "utilities.h"

void Rift::setup()
{
    points.addVertex(ofGetWidth() * .4, ofGetHeight() * .5);
    points.addVertex(ofGetWidth() * .6, ofGetHeight() * .5);
    points.close();
}

void Rift::update(double delta_t, const FlowCam& flowcam)
{
    
    for (int i = 0; i < points.size(); i ++){
        int j = (i + 1) % points.size();
        ofPoint a = points[i];
        ofPoint b = points[j];
        float d = ofDistSquared(a.x, a.y, b.x, b.y);
        if (d > max_dist_squared)
        {
            points.insertVertex((a + b) * .5 + ofPoint(ofRandomf() * 5, ofRandomf() * 5), j);
        }
        if (d < min_dist_squared)
        {
            
        }
    }
    //
    const Mat flow_high = flowcam.getFlowHigh();
    printMatrixInfo(flow_high);
    float scale_game_to_flow = (float)flow_high.cols / ofGetWidth();
    //
    for (int i = 0; i < points.size(); i ++){
        ofPoint p = points[i] * scale_game_to_flow;
        uchar flow = flow_high.at<uchar>(p.y, p.x);
        ofLogVerbose("Rift") << static_cast<unsigned>(flow);
        if (flow > 128)
        {
            points[i] += points.getNormalAtIndex(i);
        }
        else
        {
            points[i] -= .2 * points.getNormalAtIndex(i);
        }
    }
}

void Rift::draw()
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