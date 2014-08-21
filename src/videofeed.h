#ifndef SLOW_VIDEOFEED_H_
#define SLOW_VIDEOFEED_H_

#include "ofMain.h"
#include "ofxCv.h"

using namespace std;
using namespace cv;

class VideoFeed : public ofThread
{

public:
    VideoFeed() {};
    VideoFeed(const VideoFeed&) = delete;            // no copy
    VideoFeed& operator=(const VideoFeed&) = delete; // no assign

    void setAspectRatio(int new_width, int new_height)
    {
        width = new_width;
        height = new_height;
    }
    
    bool getFrame(Mat& frame)
    {
        Poco::ScopedLock<ofMutex> lock(mutex);
        int capture_width = frame_full.cols;
        int capture_height = frame_full.rows;
        float ratio = (float)width / height;
        int w = min(capture_width, static_cast<int>(round(capture_height * ratio)));
        int h = min(capture_height, static_cast<int>(round(capture_width / ratio)));
        cv::Rect roi = cv::Rect((capture_width - w) / 2, (capture_height - h) / 2, w, h);
        frame = frame_full(roi).clone();
    }

protected:
    int width, height;
    Mat frame_full;
};

class VideoFeedStatic : public VideoFeed
{
public:
    void setup(string path) ;
private:
    ofImage image;
};

class VideoFeedWebcam : public VideoFeed
{
public:

};



#endif /* defined(SLOW_VIDEOFEED_H_) */