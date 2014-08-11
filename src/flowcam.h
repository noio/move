
#ifndef SLOW_FLOWCAM_H_
#define SLOW_FLOWCAM_H_

#include "ofMain.h"
#include "ofxCv.h"

#include <iostream>

using std::string;

class FlowCam {
public:
    FlowCam(){};
    FlowCam(const FlowCam&) = delete;            // no copy
    FlowCam& operator=(const FlowCam&) = delete; // no assign
    
    void setup(int in_capture_width, int in_capture_height, int in_screen_width, int in_screen_height, int device_id, float zoom);
    void update();
    void draw(float x, float y, float width, float height);
    
    void drawDebug();
    
    cv::Size getFlowSize() const { return cv::Size(flow_width, flow_height); };
    cv::Size getFrameSize() const { return cv::Size(frame.cols, frame.rows); };
    cv::Mat getFlowHigh() const { return flow_high; };
    ofVec2f getFlowAt(float x, float y) { return ofxCv::toOf(flow.at<cv::Vec2f>(y, x)); };
    cv::Mat getFrame() const { return frame; };
    ofImage& getScreenTexture() { return frame_screen_im; };
    ofImage& getFlowHighTexture() { return flow_high_im; };
    const vector<ofPolyline>&  getContoursHigh() const { return contourfinder_high.getPolylines(); };

    void setFlowThreshold(float threshold_high);
    void setFlowErosionSize(int in_flow_erosion_size);
    void setScreenSize(int in_screen_width, int in_screen_height);
    void setCaptureSize(int in_capture_width, int in_capture_height);
    void setZoom(float in_zoom);
    void setFlip(int flip);
    bool hasData(){return has_data;};
    
    const int pyrdown_steps = 2;

    
private:
    void initGrabber(int device_id);
    void threadedFunction();
    void updateFrame();
    void updateFlow();
    void computeRoi();
    void reset();
    
    cv::Mat open_kernel;
    ofVideoGrabber camera;
    cv::Rect capture_roi;
    
    ofxCv::FlowFarneback opticalflow;
    ofxCv::ContourFinder contourfinder_low;
    ofxCv::ContourFinder contourfinder_high;
    
    cv::Mat frame_full, frame, frame_gray, frame_screen;
    cv::Mat magnitude, angle, flow, flow_high, flow_high_prev, flow_high_hist; // flow_behind, flow_new;
//    cv::Mat flow_hist;
    ofImage frame_screen_im, flow_high_im;
    float global_flow;
    int flow_creep_counter = 0;
    
    float last_capture;
    
    int capture_width, capture_height, screen_width, screen_height, flow_width, flow_height;
    int device_id;
    float zoom = 1.0;
    int flip = 1;
    float flow_threshold_high = 0.5f;
    int flow_erosion_size = 5;
    
    bool has_data = false;
};


#endif /* defined(__slow__flowcam__) */
