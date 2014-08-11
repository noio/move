#include "flowcam.h"

using namespace ofxCv;

void FlowCam::setup(int in_capture_width, int in_capture_height, int in_screen_width, int in_screen_height, int in_device_id, float in_zoom)
{
    // Initialize member variables
    capture_width = in_capture_width;
    capture_height = in_capture_height;
    screen_width = in_screen_width;
    screen_height = in_screen_height;
    device_id = in_device_id;
    zoom = in_zoom;
    setFlowErosionSize(flow_erosion_size);
    computeRoi();
    initGrabber(device_id);
    // Texture setup
    // Contourfinder setup
    contourfinder_high.setSimplify(true);
    contourfinder_high.setMinArea(80);
    contourfinder_high.getTracker().setSmoothingRate(0.2);
    contourfinder_low.setSimplify(true);
    contourfinder_low.setMinArea(80);
    contourfinder_low.getTracker().setSmoothingRate(0.2);
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

void FlowCam::draw(float x, float y, float width, float height)
{
    if (frame_screen_im.isAllocated())
    {
        frame_screen_im.draw(x, y, width, height);
    }
    else
    {
        ofClear(0, 0, 0);
        ofSetColor(255, 255, 255);
        ofDrawBitmapString("[No Camera Feed]", ofGetWidth() / 2, ofGetHeight() / 2);
    }
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


void FlowCam::setScreenSize(int in_screen_width, int in_screen_height)
{
    if (screen_width == in_screen_width && screen_height == in_screen_height) return;
    screen_width = in_screen_width;
    screen_height = in_screen_height;
    computeRoi();
}

void FlowCam::setZoom(float in_zoom)
{
    if (zoom == in_zoom) return;
    zoom = in_zoom;
    computeRoi();
}

void FlowCam::setCaptureSize(int in_capture_width, int in_capture_height)
{
    if (capture_width == in_capture_width && capture_height == in_capture_height) return;
    capture_width = in_capture_width;
    capture_height = in_capture_height;
    computeRoi();
    initGrabber(device_id);
}

void FlowCam::setFlowErosionSize(int in_flow_erosion_size)
{
    flow_erosion_size = in_flow_erosion_size;
    open_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                            cv::Size(2 * flow_erosion_size + 1, 2 * flow_erosion_size + 1),
                                            cv::Point(flow_erosion_size, flow_erosion_size));
}

void FlowCam::setFlowThreshold(float threshold_high)
{
    flow_threshold_high = threshold_high;
}

void FlowCam::setFlip(int in_flip)
{
    flip = in_flip;
}


////////// PRIVATE METHODS //////////

void FlowCam::initGrabber(int device_id)
{
    if (camera.isInitialized())
    {
        ofLogWarning("FlowCam") << "Camera already inited";
        camera.close();
    }

    if (camera.listDevices().size() > 1)
    {
        camera.setDeviceID(device_id);
    }
    camera.setUseTexture(true);
    camera.initGrabber(capture_width, capture_height);
    frame_full = toCv(camera.getPixelsRef());
}

//void FlowCam::threadedFunction()
//{
//    while (isThreadRunning())
//    {
//        update();
//    }
//}

void FlowCam::update()
{
    if (camera.isInitialized())
    {
        camera.update();

        if (camera.isFrameNew())
        {
            updateFrame();
            updateFlow();
            last_capture = ofGetElapsedTimef();
        }
    }
}


void FlowCam::updateFrame()
{
    if (flip < 2)
    {
        cv::flip(frame_full(capture_roi), frame, flip);
    }
    else
    {
        frame = frame_full(capture_roi);
    }

//    frame_screen = toCv(frame_screen_im);
    cv::resize(frame, frame_screen, cv::Size(screen_width, screen_height), 0, 0, cv::INTER_NEAREST);
    toOf(frame_screen, frame_screen_im);
    
    cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);

    for (int i = 0; i < pyrdown_steps; i++)
    {
        cv::pyrDown(frame_gray, frame_gray);
    }

    assert(frame_gray.cols == flow_width && frame_gray.rows == flow_height);
    frame_screen_im.update();
}

void FlowCam::updateFlow()
{
    float since_last_capture = ofGetElapsedTimef() - last_capture;
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
    // Compute the low speed mask
    // Shadow these variables:
    float adj_flow_threshold_high = flow_threshold_high * since_last_capture * 30.0;
    //
    // Compute the high speed mask
    flow_high = magnitude > adj_flow_threshold_high; // & flow_low_prev > 0;
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
    toOf(flow_high_hist, flow_high_im);
}



void FlowCam::computeRoi()
{
    float ratio = (float)screen_width / screen_height;
    int w = std::min(capture_width, static_cast<int>(round(capture_height * ratio)));
    int h = std::min(capture_height, static_cast<int>(round(capture_width / ratio)));
    w /= zoom;
    h /= zoom;
    capture_roi = cv::Rect((capture_width - w) / 2, (capture_height - h) / 2, w, h);
    flow_width = ceil(w / pow(2.0, pyrdown_steps));
    flow_height = ceil(h / pow(2.0, pyrdown_steps));
    reset();
}

