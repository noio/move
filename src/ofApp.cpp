#include "ofApp.h"
#include "ofxCv.h"
#include "ofxRemoteUIServer.h"
#include "utilities.h"

using namespace ofxDS;

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
    ofSetFrameRate(60);
    ofSetLogLevel(OF_LOG_NOTICE);
    loadConfig();
    //
    setupUI();
    //===== REMOTE CAM SETUP =====
    rgb_there = ofPtr<VideoFeed>(setupVideoFeed(rgb_there_source));
    rgb_there->setFlip(rgb_there_flip);
    rgb_there->setMaxFPS(rgb_there_fps);
    //===== LOCAL CAMERA SETUP =====
    rgb_here = ofPtr<VideoFeed>(setupVideoFeed(rgb_here_source));
    rgb_here->setFlip(rgb_here_flip);
    rgb_here->setMaxFPS(rgb_there_fps);
    // Window setup
    ofSetWindowPosition(window_x, window_y);
    ofSetWindowShape(window_width, window_height);
    //
    skeletonfeed = ofPtr<SkeletonFeed>(new SkeletonFeed());
    if (use_skeletons)
    {
        skeletonfeed->setup("http://" + config["local"]["server"].asString() + "/activeskeletonsprojected");
    }
    // Numbers below found by trial & error
    // Frameworks till does crazy squishing at non-standard resolutions
    // so there is no making sense of this.
    skeletonfeed->setOutputScaleAndOffset(ofPoint(896, 400, 1), ofPoint(0, 0, 1));
    //
    flowcam_here.setup(160);
    flowcam_there.setup(160);
    //
    contourfinder.setSimplify(true);
    contourfinder.setMinArea(80);
    contourfinder.getTracker().setSmoothingRate(0.2);
    //
    lights.setup(5);
}

void ofApp::loadConfig()
{
    const string configpath = ofBufferFromFile("configpath").getText();
    if (!config.open(configpath))
    {
        ofLogError("ofApp") << "Failed to open config file";
        ofSystemAlertDialog("Failed to load config file at " + configpath);
    }
    else
    {
        ofLogNotice("ofApp") << "Loaded config: \n" << config.toStyledString();
    }

}

void ofApp::setupUI()
{
    // SETUP UI
    RUI_SETUP(44040); //start server
    RUI_NEW_GROUP("Reboot Required");
    RUI_SHARE_PARAM(window_x, 0, ofGetScreenWidth() / 2);
    RUI_SHARE_PARAM(window_y, 0, ofGetScreenHeight() / 2);
    RUI_SHARE_PARAM(window_width, 320, 1920);
    RUI_SHARE_PARAM(window_height, 240, 1080);
    vector<string> menuItems;
    menuItems.push_back("PLACEHOLDER");
    menuItems.push_back("WEBCAM0");
    menuItems.push_back("WEBCAM1");
    menuItems.push_back("SERVER_LOCAL");
    menuItems.push_back("SERVER_REMOTE");
    menuItems.push_back("CUSTOM_URL");
    RUI_SHARE_ENUM_PARAM(rgb_here_source, 0, 5, menuItems);
    RUI_SHARE_ENUM_PARAM(rgb_there_source, 0, 5, menuItems);
    RUI_SHARE_PARAM(source_custom_url);
    RUI_SHARE_PARAM(use_skeletons);
    RUI_SHARE_PARAM(rgb_here_flip, -1, 2);
    RUI_SHARE_PARAM(rgb_there_flip, -1, 2);
    RUI_SHARE_PARAM(rgb_here_fps, 1, 30);
    RUI_SHARE_PARAM(rgb_there_fps, 1, 30);
    RUI_NEW_GROUP("Debug");
    RUI_SHARE_PARAM(draw_debug);
    RUI_SHARE_PARAM(disable_local_rgb);
    menuItems.clear();
    menuItems.push_back("NONE");
    menuItems.push_back("FLOW_HERE");
    menuItems.push_back("FLOW_THERE");
    menuItems.push_back("FLOWHIST_HERE");
    menuItems.push_back("FLOWHIST_THERE");
    RUI_SHARE_ENUM_PARAM(debug_overlay, 0, 4, menuItems);
    RUI_NEW_GROUP("Creation");
    RUI_SHARE_PARAM(max_rifts, 0, 10);
    RUI_SHARE_PARAM(new_rift_min_flow, 0, 255);
    RUI_SHARE_PARAM(create_rifts_time, 0, 10);
    RUI_NEW_GROUP("Graphics");
    RUI_SHARE_COLOR_PARAM(rgb_here_multiply);
    RUI_SHARE_PARAM(flow_hist_darkness, 0, 1.0);
    RUI_SHARE_PARAM(Rift::inner_light_strength, 0, 10000.0f);
    RUI_NEW_GROUP("Rift");
    RUI_SHARE_PARAM(Rift::fade_max_area, 200, 500);
    RUI_SHARE_PARAM(Rift::fade_out_time, 5.0, 50);
    RUI_SHARE_PARAM(Rift::fade_in_time, 0.1, 5.0);
    RUI_SHARE_PARAM(Rift::open_time, 1, 10);
    RUI_SHARE_PARAM(Rift::resample_time, 2, 60.0);
    RUI_SHARE_PARAM(Rift::max_point_dist, 10, 100);
    RUI_SHARE_PARAM(Rift::grow_speed, 0, 4);
    RUI_SHARE_PARAM(Rift::grow_min_flow_squared, 0, 10);
    RUI_SHARE_PARAM(Rift::shrink_speed, 0, 4);
    RUI_SHARE_PARAM(Rift::shrink_delay, 0, 10);
    RUI_NEW_GROUP("Lights");
    RUI_SHARE_PARAM(lights.ray_length, 0, 2.0f);
    RUI_SHARE_PARAM(lights.ray_rift_normal_bias, 0, 40.0f);
    //load values from XML, as they were last saved (if they were)
    RUI_LOAD_FROM_XML();
}

VideoFeed* ofApp::setupVideoFeed(VideoSource source)
{
    VideoFeed* feed;
    switch (source)
    {
        case VIDEO_SOURCE_PLACEHOLDER:
        {
            feed = new VideoFeedStatic();
            ((VideoFeedStatic *)feed)->setup("stockholm.jpg");
            break;
        }
        case VIDEO_SOURCE_WEBCAM0:
        {
            feed = new VideoFeedWebcam();
            ((VideoFeedWebcam *)feed)->setup(0, WEBCAM_RES_720);
            break;
        }
        case VIDEO_SOURCE_WEBCAM1:
        {
            feed = new VideoFeedWebcam();
            ((VideoFeedWebcam *)feed)->setup(1, WEBCAM_RES_720);
            break;
        }
        case VIDEO_SOURCE_SERVER_LOCAL:
        {
            VideoFeedImageURL* f = new VideoFeedImageURL();
            f->setup("http://" + config["local"]["server"].asString() + "/color");
            feed = f;
            break;
        }
        case VIDEO_SOURCE_SERVER_REMOTE:
        {
            VideoFeedImageURL* f = new VideoFeedImageURL();
            f->setup("http://" + config["local"]["server"].asString() + "/color");
            feed = f;
            break;
        }

        case VIDEO_SOURCE_CUSTOM_URL:
        {
            VideoFeedImageURL* f = new VideoFeedImageURL();
            f->setup(source_custom_url);
            feed = f;
            break;
        }
        default:
            break;
    }
    feed->setAspectRatio(ofGetWidth(), ofGetHeight());
    return feed;
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
    ofLogNotice("ofApp") << "resized " << w << "x" << h;
    window_width = w;
    window_height = h;
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
