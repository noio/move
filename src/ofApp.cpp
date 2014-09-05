#include "ofApp.h"
#include "ofxCv.h"
#include "ofxRemoteUIServer.h"
#include "utilities.h"

void maskBeginAlpha()
{
    ofFill();
    glColorMask(false, false, false, true);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void maskBeginContent()
{
    glColorMask(true, true, true, true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

void maskEnd()
{
    glDisable(GL_BLEND);
}

void drawMatFull(const Mat& matrix)
{
    if (!matrix.empty())
    {
        cv::Mat rgb;
        if (matrix.type() == CV_32FC2)
        {
            float maxval = matrix.cols / 5;
            vector<Mat> channels;
            split(matrix, channels);
            channels.push_back(Mat::zeros(matrix.rows, matrix.cols, CV_32F));
            merge(channels, rgb);
            rgb.convertTo(rgb, CV_8UC3, 255 / maxval, 128);
        }
        else
        {
            rgb = matrix;
        }
        drawMat(rgb, 0, 0, ofGetWidth(), ofGetHeight());
    }
}


//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    skeletonfeed = ofPtr<SkeletonFeed>(new SkeletonFeed());
    skeletonfeed->setup("http://192.168.1.34:1338/activeskeletonsprojected");
    // Numbers below found by trial & error
    // Frameworks till does crazy squishing at non-standard resolutions
    // so there is no making sense of this.
    skeletonfeed->setOutputScaleAndOffset(ofPoint(896, 400, 1), ofPoint(0, 0, 1));
    //
    flowcam_here.setup(160);
    flowcam_there.setup(160);
    //
    //===== REMOTE CAM SETUP =====
    VideoFeedStatic* rgb_there_p = new VideoFeedStatic();
    rgb_there_p->setup("stockholm.jpg");
//    VideoFeedWebcam* rgb_there_p = new VideoFeedWebcam();
//    rgb_there_p->setup(1, 1280, 720);
//    VideoFeedImageUrl* rgb_there_p = new VideoFeedImageUrl();
//    rgb_there_p->setup("http://192.168.1.34:1338/color");
    rgb_there_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_there = ofPtr<VideoFeed>(rgb_there_p);
    //
    //===== LOCAL CAMERA SETUP =====
    VideoFeedWebcam* rgb_here_p = new VideoFeedWebcam();
    rgb_here_p->setup(0, WEBCAM_RES_720);
//    VideoFeedImageUrl* rgb_here_p = new VideoFeedImageUrl();
//    rgb_here_p->setup("http://192.168.1.34:1338/color");
//    VideoFeedStatic* rgb_here_p = new VideoFeedStatic();
//    rgb_here_p->setup("denhaag.jpg");
    rgb_here_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_here = ofPtr<VideoFeed>(rgb_here_p);
    //
    contourfinder.setSimplify(true);
    contourfinder.setMinArea(80);
    contourfinder.getTracker().setSmoothingRate(0.2);
    //
    lights.setup(5);
    // SETUP UI
    OFX_REMOTEUI_SERVER_SETUP(44040); //start server
    OFX_REMOTEUI_SERVER_NEW_GROUP("Global");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(draw_debug);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(disable_local_rgb);
    vector<string> menuItems;
    menuItems.push_back("NONE");
    menuItems.push_back("FLOW_HERE");
    menuItems.push_back("FLOW_THERE");
    menuItems.push_back("FLOWHIST_HERE");
    menuItems.push_back("FLOWHIST_THERE");
    OFX_REMOTEUI_SERVER_SHARE_ENUM_PARAM(debug_overlay, 0, 4, menuItems);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Creation");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(max_rifts, 0, 10);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(new_rift_min_flow, 0, 255);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(create_rifts_time, 0, 10);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Graphics");
    OFX_REMOTEUI_SERVER_SHARE_COLOR_PARAM(rgb_here_multiply);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(flow_hist_darkness, 0, 1.0);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Rift");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::inner_light_strength, 0, 10000.0f);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::fade_max_area, 200, 500);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::fade_out_time, 5.0, 50);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::fade_in_time, 0.1, 5.0);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::resample_time, 2, 60.0);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::max_point_dist, 10, 100);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::grow_speed, 0, 2);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::grow_min_flow_squared, 0, 10);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::shrink_speed, 0, 2);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::shrink_delay, 0, 10);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Lights");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(lights.ray_length, 0, 2.0f);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(lights.ray_rift_normal_bias, 0, 40.0f);
    //load values from XML, as they were last saved (if they were)
    OFX_REMOTEUI_SERVER_LOAD_FROM_XML();
}

//--------------------------------------------------------------
void ofApp::update()
{
    delta_t = ofGetLastFrameTime();
    cv::Mat frame;
    if (rgb_here->getFrame(frame))
    {
        flowcam_here.update(frame);
    }
    if (rgb_there->getFrame(frame))
    {
        flowcam_there.update(frame);
    }
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].update(delta_t, flowcam_here, flowcam_there);
        if (rifts[i].fade <= 0)
        {
            rifts.erase(rifts.begin() + i);
            i--;
        }
    }
    //
    updateFlowHist();
    //
    create_rifts_timer += delta_t;
    if (create_rifts_timer > create_rifts_time)
    {
        createRifts();
        create_rifts_timer = 0;
    }
    vector<Skeleton> skeletons = skeletonfeed->getSkeletons();
    lights.update(delta_t, skeletons);
}

void ofApp::updateFlowHist()
{
    flow_here_hist = flowcam_here.getFLowHighHist();
    flow_there_hist = flowcam_there.getFLowHighHist();
    //
    if (flow_here_hist.empty() && flow_there_hist.empty())
    {
        return;
    }
    if (flow_here_hist.empty())
    {
        flow_hist_threshold = flow_there_hist > new_rift_min_flow;
    }
    else if (flow_there_hist.empty())
    {
        flow_hist_threshold = flow_here_hist > new_rift_min_flow;
    }
    else
    {
        if (flow_there_hist.size() != flow_here_hist.size())
        {
            cv::resize(flow_there_hist, flow_there_hist, flow_here_hist.size());
        }
        flow_hist_threshold = (flow_here_hist > new_rift_min_flow) | (flow_there_hist > new_rift_min_flow);
    }
    if (!flow_hist_threshold.empty())
    {
        contourfinder.findContours(flow_hist_threshold);
    }
}

void ofApp::createRifts()
{
    if (rifts.size() < max_rifts)
    {
        if (contourfinder.size())
        {
            float scale_flow_to_game = ofGetWidth() / (float)flow_hist_threshold.cols;
            int max_area = 0;
            int biggest_contour_idx = 0;
            for (int i = 0; i < contourfinder.size(); i ++)
            {
                int area = contourfinder.getBoundingRect(i).area();
                if (area > max_area)
                {
                    biggest_contour_idx = i;
                    max_area = area;
                }
            }
            const ofPolyline& contour = contourfinder.getPolyline(biggest_contour_idx);
            ofRectangle bbc = contour.getBoundingBox();
            bbc.scale(scale_flow_to_game);
            bbc.x *= scale_flow_to_game;
            bbc.y *= scale_flow_to_game;
            for (int i = 0; i < rifts.size(); i++)
            {
                const ofRectangle bbr = rifts[i].getBoundingBox();
                if (bbc.intersects(bbr))
                {
                    return;
                }
            }
            Rift r;
            r.setup(bbc);
            rifts.push_back(r);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    if (!disable_local_rgb)
    {
        ofSetColor(rgb_here_multiply);
        rgb_here->draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    else
    {
        rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    if (!flow_here_hist.empty())
    {
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        cv::Mat flipped = 255 - flow_here_hist * flow_hist_darkness;
        drawMat(flipped, 0, 0, ofGetWidth(), ofGetHeight());
        ofDisableBlendMode();
        ofSetColor(ofColor::white);
    }
    //
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawOutline();
    }
    //
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawLights(lights);
    }
    maskBeginAlpha();
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawMask();
    }
    maskBeginContent();
    rgb_there->draw(0, 0, ofGetWidth(), ofGetHeight());
    maskEnd();
    //
    for (int i = 0; i < rifts.size(); i ++)
    {
        rifts[i].drawInnerLight();
    }
    //
    if (draw_debug)
    {
        ofSetColor(255, 200);
        ofEnableAlphaBlending();
        switch (debug_overlay)
        {
        case DEBUGOVERLAY_FLOW_HERE:
            drawMatFull(flowcam_here.getFlow());
            break;
        case DEBUGOVERLAY_FLOW_THERE:
            drawMatFull(flowcam_there.getFlow());
            break;
        case DEBUGOVERLAY_FLOWHIST_HERE:
            drawMatFull(flowcam_here.getFLowHighHist());
            break;
        case DEBUGOVERLAY_FLOWHIST_THERE:
            drawMatFull(flowcam_there.getFLowHighHist());
            break;
        default:
            break;
        }
        ofDisableAlphaBlending();
        for (int i = 0; i < rifts.size(); i ++)
        {
            rifts[i].drawDebug();
        }
        lights.drawDebug();
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(), 0) + "fps (r11)", ofPoint(3, 13));
        ofDrawBitmapStringHighlight("[d]ebug view", ofPoint(3, 33));
        float scale_flow_to_game = ofGetWidth() / (float)flowcam_here.getFlowHigh().cols;
        ofPushMatrix();
        ofScale(scale_flow_to_game, scale_flow_to_game);
        for (int i = 0; i < contourfinder.size(); i ++)
        {
            contourfinder.getPolyline(i).getSmoothed(3).draw();
        }
        ofPopMatrix();
        skeletonfeed->drawDebug();
    }
}


void ofApp::exit()
{
    rgb_here.reset();
    rgb_there.reset();
    skeletonfeed.reset();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch (key)
    {
    case 'd':
        draw_debug = !draw_debug;
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
    rgb_here->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_there->setAspectRatio(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
