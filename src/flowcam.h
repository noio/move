
#ifndef MOVE_FLOWCAM_H_
#define MOVE_FLOWCAM_H_

#include "ofMain.h"
#include "ofxCv.h"

#include <iostream>

using namespace std;
using namespace cv;

class FlowCam
{
public:
    FlowCam() {};
    FlowCam(const FlowCam&) = delete;            // no copy
    FlowCam& operator=(const FlowCam&) = delete; // no assign

    void setup(int max_flow_width);
    void update(Mat frame, double delta_t);

    void drawDebug();

    cv::Mat getFlowHigh() const { return flow_high; };
    ofVec2f getFlowAt(float x, float y) const { return ofxCv::toOf(flow.at<cv::Vec2f>(y, x)); };
    const vector<ofPolyline>&  getContoursHigh() const { return contourfinder_high.getPolylines(); };

    void setFlowThreshold(float threshold_high) { flow_threshold_high = threshold_high; };
    void setFlowErosionSize(int in_flow_erosion_size);
    bool hasData() { return has_data; };

private:
    void reset();

    cv::Mat open_kernel;

    ofxCv::FlowFarneback opticalflow;
    ofxCv::ContourFinder contourfinder_high;

    cv::Mat frame_gray, frame_screen;
    cv::Mat magnitude, angle, flow, flow_high, flow_high_prev, flow_high_hist;

    float global_flow;
    int flow_creep_counter = 0;

    int max_flow_width = 240;
    float flow_threshold_high = 0.5f;
    int flow_erosion_size;

    bool has_data = false;
};


#endif /* defined(MOVE_FLOWCAM_H_) */
