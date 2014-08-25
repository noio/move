#include "flowcam.h"

using namespace ofxCv;

void FlowCam::setup(int max_flow_width)
{
    // Initialize member variables
    setFlowErosionSize(5);
    // Contourfinder setup
    contourfinder_high.setSimplify(true);
    contourfinder_high.setMinArea(80);
    contourfinder_high.getTracker().setSmoothingRate(0.2);
    // OpticalFlow setup
    opticalflow.setPyramidScale(0.5);
    opticalflow.setNumLevels(2);
    opticalflow.setWindowSize(7);
    opticalflow.setNumIterations(3);
    opticalflow.setPolyN(5);
    opticalflow.setPolySigma(1.2);
    //
    reset();
}

void FlowCam::reset()
{
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
    for (int i = 0; i < contourfinder_high.size(); i ++)
    {
        contourfinder_high.getPolyline(i).draw();
    }
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

void FlowCam::update(cv::Mat frame, double delta_t)
{
    cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
    while (frame_gray.cols > max_flow_width)
    {
        cv::pyrDown(frame_gray, frame_gray);
    }
    opticalflow.calcOpticalFlow(frame_gray);
    flow = opticalflow.getFlow();
    std::swap(flow_high_prev, flow_high);
    // ofxCV wrapper returns a 1x1 flow image after the first optical flow computation.
    if (flow.cols == 1)
    {
        flow = cv::Mat::zeros(frame_gray.rows, frame_gray.cols, CV_32FC2);
        flow_high_prev = cv::Mat::zeros(flow.rows, flow.cols, CV_8U);
        flow_high_hist = cv::Mat::zeros(flow.rows, flow.cols, CV_8U);
    }
    std::vector<cv::Mat> xy(2);
    cv::split(flow, xy);
    cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);
    //
    float adj_flow_threshold_high = flow_threshold_high * delta_t * 30.0;
    //
    // Compute the high speed mask
    flow_high = magnitude > adj_flow_threshold_high;
    cv::erode(flow_high, flow_high, open_kernel);
    cv::dilate(flow_high, flow_high, open_kernel);
    // Update history
    flow_high_hist += flow_high / 16;
    flow_high_hist -= 1;
    // Check for flow creep
    global_flow = cv::sum(flow_high)[0] / 255 / (float)(flow_high.cols * flow_high.rows);
    if (global_flow > 0.2f)
    {
        flow_creep_counter ++;
        if (flow_creep_counter > 1000)
        {
            ofLogNotice("FlowCam") << "flow creep detected; resetting";
            reset();
        }
    }
    else
    {
        flow_creep_counter = MAX(0, flow_creep_counter - 1);
    }
    contourfinder_high.findContours(flow_high);
}
