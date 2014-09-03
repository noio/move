#include "skeletonfeed.h"

const int SkeletonFeed::SPINE = 1;
const int SkeletonFeed::HEAD = 3;
const int SkeletonFeed::HAND_LEFT = 7;
const int SkeletonFeed::HAND_RIGHT = 11;


void SkeletonFeed::setMaxFps(float in_fps)
{
    lock();
    wait_millis = 1000.0 / in_fps;
    unlock();
}

void SkeletonFeed::setInputScale(float scale_x, float scale_y)
{
    lock();
    input_scale = ofPoint(scale_x, scale_y, 1.0);
    unlock();
}

void SkeletonFeed::setInputScale(ofPoint scale)
{
    lock();
    input_scale = scale;
    unlock();
}

/*
 * Set the top-left point and size of the rectangle that the skeletons
 * are projected to
 */
void SkeletonFeed::setOutputScaleAndOffset(ofPoint scale, ofPoint offset)
{
    lock();
    output_scale = scale;
    output_offset = offset;
    unlock();
}

/*
 * This assumes a skeleton feed that, like on Kinect, maps 1:1 to the RGB
 * pixels. Otherwise, use setOutputScaleAndOffset() to manually set projection.
 */
void SkeletonFeed::setOutputFillScreen()
{
    float ratio = (float)input_scale.x / input_scale.y;
    int w = MAX(ofGetWidth(), static_cast<int>(round(ofGetHeight() * ratio)));
    int h = MAX(ofGetHeight(), static_cast<int>(round(ofGetWidth() / ratio)));
    setOutputScaleAndOffset(ofPoint(w, h), ofPoint(-w / 2, -h / 2));
}

void SkeletonFeed::setup(string in_url)
{
    url = in_url;
    setInputScale(640, 480);
    setOutputFillScreen();
    startThread(true, false);
}

ofPoint SkeletonFeed::getPoint(Json::Value point)
{
    return ofPoint(point["X"].asFloat() / input_scale.x,
                   point["Y"].asFloat() / input_scale.y,
                   point["Z"].asFloat());
}


void SkeletonFeed::threadedFunction()
{
    while (isThreadRunning())
    {
        double fetch_start = ofGetElapsedTimeMillis();
        if (json.open(url))
        {
            if (json.isMember("Skeletons") && json["Skeletons"].isArray())
            {
                lock();
                skeletons.resize(json["Skeletons"].size());
                for (int i = 0; i < json["Skeletons"].size(); i ++)
                {
                    Json::Value joints = json["Skeletons"][i]["Skeleton"]["Joints"];
                    skeletons[i].location = json["Skeletons"][i]["Location"].asInt();
                    skeletons[i].head = getPoint(joints[HEAD]);
                    skeletons[i].hand_left = getPoint(joints[HAND_LEFT]);
                    skeletons[i].hand_right = getPoint(joints[HAND_RIGHT]);
                    skeletons[i].spine = getPoint(joints[SPINE]);
                }
                unlock();
            }
        }
        double elapsed = ofGetElapsedTimeMillis() - fetch_start;
        double wait = wait_millis - elapsed;
        if (wait_millis > 0)
        {
            ofLogVerbose("SkeletonFeed") << "Sleeping " << wait_millis << " ms";
            ofSleepMillis(wait_millis);
        }
    }
}

void SkeletonFeed::drawDebug()
{
    ofSetLineWidth(2.0f);
    ofNoFill();
    ofPoint output_size = ofPoint(ofGetWidth(), ofGetHeight());
    for (int i = 0; i < skeletons.size(); i++) {
        ofSetColor(ofColor::fromHsb(190 * i, 255, 255), 255);
        ofPoint hd = skeletons[i].head * output_size;
        ofPoint hl = skeletons[i].hand_left * output_size;
        ofPoint hr = skeletons[i].hand_right * output_size;
        ofDrawBitmapStringHighlight(ofToString(i), hd);
        ofSetCircleResolution(6);
        ofCircle(hd, 20);
        ofDrawBitmapStringHighlight(ofToString(i), hl);
        ofSetCircleResolution(4);
        ofCircle(hl, 20);
        ofDrawBitmapStringHighlight(ofToString(i), hr);
        ofSetCircleResolution(4);
        ofCircle(hr, 20);
    }
}