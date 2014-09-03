#include "flowcam.h"

using namespace ofxCv;

void FlowCam::setup(int max_flow_width)
{
    // Initialize member variables
    setFlowErosionSize(5);
    // OpticalFlow setup
    opticalflow.setPyramidScale(0.5);
    opticalflow.setNumLevels(2);
    opticalflow.setWindowSize(7);
    opticalflow.setNumIterations(3);
    opticalflow.setPolyN(5);
    opticalflow.setPolySigma(1.2);
    //
    flow = cv::Mat::zeros(2, 2, CV_32FC2);
    //
    reset();
}

void FlowCam::reset()
{
    flow_creep_counter = 0;
    opticalflow.resetFlow();
    has_data = false;
}

void FlowCam::drawDebug()
{
    ofPushStyle();
    // Draw the optical flow maps
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(224, 160, 58, 128);
    ofxCv::drawMat(flow_high, 0, 0, ofGetWidth(), ofGetHeight());
    ofDisableBlendMode();
    ofPushMatrix();
    ofSetLineWidth(4.0);
    ofSetColor(ofColor::red);
    ofScale(ofGetWidth() / (float)flow_high.cols, ofGetHeight() / (float)flow_high.rows);
    ofPopMatrix();
    ofPopStyle();
}

void FlowCam::setFlowErosionSize(int in_flow_erosion_size)
{
    flow_erosion_size = in_flow_erosion_size;
    open_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                            cv::Size(2 * flow_erosion_size + 1, 2 * flow_erosion_size + 1),
                                            cv::Point(flow_erosion_size, flow_erosion_size));
}


////////// PRIVATE METHODS //////////

void FlowCam::update(cv::Mat frame)
{
    float delta_t = ofGetElapsedTimef() - last_update;
    cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
    while (frame_gray.cols > max_flow_width)
    {
        cv::pyrDown(frame_gray, frame_gray);
    }
    opticalflow.calcOpticalFlow(frame_gray);
    flow = opticalflow.getFlow();
    std::swap(flow_high_prev, flow_high);
    // ofxCV wrapper returns a 1x1 flow image after the first optical flow computation.
    if (frame_gray.cols != flow.cols || frame_gray.rows != flow.rows)
    {
        ofLogVerbose("FlowCam") << "Initialize flow data";
        flow = cv::Mat::zeros(frame_gray.rows, frame_gray.cols, CV_32FC2);
        flow_high_prev = cv::Mat::zeros(flow.rows, flow.cols, CV_8U);
        opticalflow.resetFlow();
        opticalflow.calcOpticalFlow(frame_gray);
    }
    //
    std::vector<cv::Mat> xy(2);
    cv::split(flow, xy);
    cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);
    //
    // Compute the high speed mask
    flow_high = magnitude > flow_threshold_high;
    cv::erode(flow_high, flow_high, open_kernel);
    cv::dilate(flow_high, flow_high, open_kernel);
    // Update history
    if (flow_high_hist.size() != flow_high.size())
    {
        flow_high_hist = cv::Mat::zeros(flow_high.rows, flow_high.cols, CV_8U);
    }
    flow_high_hist += flow_high * (delta_t * 2);
    flow_high_hist -= 1;
    ofxCv::blur(flow_high_hist, flow_high_hist, 3);
    //
    flow_high_new = flow_high & ~flow_high_prev;
    // Check for flow creep
    global_flow = cv::sum(flow_high)[0] / 255 / (float)(flow_high.cols * flow_high.rows);
    if (global_flow > 0.2f)
    {
        flow_creep_counter ++;
        if (flow_creep_counter > 100)
        {
            ofLogNotice("FlowCam") << "flow creep detected; resetting";
            reset();
        }
    }
    else
    {
        flow_creep_counter = MAX(0, flow_creep_counter - 1);
    }
    has_data = true;
    last_update = ofGetElapsedTimef();
}
