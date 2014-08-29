#include "ofApp.h"
#include "ofxCv.h"
#include "ofxRemoteUIServer.h"

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


//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    flowcam_here.setup(160);
    flowcam_there.setup(160);
    //
    VideoFeedImageUrl* rgb_there_p = new VideoFeedImageUrl();
    rgb_there_p->setup("http://192.168.1.34:1338/color");
//    VideoFeedStatic* rgb_there_p = new VideoFeedStatic();
//    rgb_there_p->setup("stockholm.jpg");
    rgb_there_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_there = ofPtr<VideoFeed>(rgb_there_p);
    //
    VideoFeedWebcam* rgb_here_p = new VideoFeedWebcam();
    rgb_here_p->setup(0, 1280, 720);
    rgb_here_p->setAspectRatio(ofGetWidth(), ofGetHeight());
    rgb_here = ofPtr<VideoFeed>(rgb_here_p);
    //
    contourfinder.setThreshold(1);
    contourfinder.setSimplify(true);
    contourfinder.setMinArea(80);
    contourfinder.getTracker().setSmoothingRate(0.2);
    //
    for (int i = 0; i < 5; i ++)
    {
        Light l;
        l.setup();
        lights.push_back(l);
    }
    // SETUP UI
    OFX_REMOTEUI_SERVER_SETUP(44040); //start server
    OFX_REMOTEUI_SERVER_NEW_GROUP("Global");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(draw_debug);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(max_rifts, 0, 10);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(new_rift_min_flow, 0, 512);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(create_rifts_time, 0, 10);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Rift");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::inner_light_strength, 0, 10000.0f);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::fade_max_area, 200, 500);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::fade_time, 5, 50);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::min_age, 1, 5);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::resample_time, 2, 60.0);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::max_point_dist, 10, 100);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::grow_speed, 0, 2);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::grow_min_flow_squared, 0, 10);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::shrink_speed, 0, 2);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::tear_heat, 1, 20);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Rift::heat_decay, 0.98, 0.999);
    OFX_REMOTEUI_SERVER_NEW_GROUP("Lights");
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Light::ray_length, 0, 2.0f);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(Light::ray_rift_normal_bias, 0, 40.0f);
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
        flowcam_here.update(frame, delta_t);
    }
    if (rgb_there->getFrame(frame))
    {
        flowcam_there.update(frame, delta_t);
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
    create_rifts_timer += delta_t;
    if (create_rifts_timer > create_rifts_time)
    {
        createRifts();
        create_rifts_timer = 0;
    }
    for (int i = 0; i < lights.size(); i++)
    {
        lights[i].update(delta_t);
    }
}

void ofApp::createRifts()
{
    if (rifts.size() < max_rifts)
    {
        Mat flow_here_hist = flowcam_here.getFLowHighHist();
        Mat flow_there_hist = flowcam_there.getFLowHighHist();
        if (flow_here_hist.empty() && flow_there_hist.empty())
        {
            return;
        }
        if (flow_here_hist.empty())
        {
            flow_hist_total = cv::Mat::zeros(flow_there_hist.rows, flow_there_hist.cols, CV_32FC2);
        }
        else
        {
            flow_hist_total = flow_here_hist.clone();
        }
        //
        if (!flow_there_hist.empty())
        {
            if (flow_there_hist.size() != flow_here_hist.size())
            {
                cv::resize(flow_there_hist, flow_there_hist, flow_here_hist.size());
            }
            flow_hist_total += flow_there_hist;
        }
        
        contourfinder.findContours(flow_hist_total > new_rift_min_flow);
        if (contourfinder.size())
        {
            float scale_flow_to_game = ofGetWidth() / (float)flow_hist_total.cols;
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
            ofPolyline initial;
            // Contour polys are reverse-wound
            float f = ofRandomuf();
            const int num_pts = 10;
            for(int i = 0; i < num_pts; i ++)
            {
                initial.addVertex(contour.getPointAtPercent(f) * scale_flow_to_game);
                f -= 0.5 / num_pts;
            }
            Rift r;
            r.setup(initial);
            rifts.push_back(r);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetColor(128);
    rgb_here->draw(0, 0, ofGetWidth(), ofGetHeight());
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
        for (int i = 0; i < rifts.size(); i ++)
        {
            rifts[i].drawDebug();
        }
        for (int i = 0; i < lights.size(); i ++)
        {
            lights[i].drawDebug();
        }
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(), 0) + "fps (r11)", ofPoint(3, 13));
        ofDrawBitmapStringHighlight("[d]ebug view", ofPoint(3, 33));
        float scale_flow_to_game = ofGetWidth() / (float)flowcam_here.getFlowHigh().cols;
        ofPushMatrix();
        ofScale(scale_flow_to_game, scale_flow_to_game);
        if (flow_hist_total.cols > 0)
        {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            drawMat(flow_hist_total, 0, 0);
            ofDisableBlendMode();
        }
        contourfinder.draw();
        ofPopMatrix();
    }
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
