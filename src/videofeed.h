#ifndef SLOW_VIDEOFEED_H_
#define SLOW_VIDEOFEED_H_

#include "ofMain.h"
#include "ofxCv.h"

using namespace std;
using namespace cv;
using namespace ofxCv;

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

    void setFlip(int new_flip)
    {
        flip = new_flip;
    }

    bool getFrame(Mat& frame)
    {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if (!frame_im.isAllocated())
        {
            frame = cv::Mat::zeros(width,height, CV_8UC3);
            return false;
        }
        frame_mat = toCv(frame_im);
        int capture_width = frame_mat.cols;
        int capture_height = frame_mat.rows;
        float ratio = (float)width / height;
        int w = min(capture_width, static_cast<int>(round(capture_height * ratio)));
        int h = min(capture_height, static_cast<int>(round(capture_width / ratio)));
        roi = cv::Rect((capture_width - w) / 2, (capture_height - h) / 2, w, h);
        if (flip < 2)
        {
            cv::flip(frame_mat(roi), frame, flip);
        }
        else
        {
            frame = frame_mat(roi).clone();
        }
        bool frame_was_new = frame_is_new;
        frame_is_new = false;
        return frame_was_new;
    }

protected:
    int width, height, flip = 1;
    bool frame_is_new = false;
    cv::Rect roi;
    Mat frame_mat;
    ofImage frame_im;
};

class VideoFeedStatic : public VideoFeed
{
public:
    void setup(string path) ;
};

class VideoFeedWebcam : public VideoFeed
{
public:
    void setup(int device, int capture_width, int capture_height) ;
    void threadedFunction() ;
private:
    ofVideoGrabber camera;
};



#endif /* defined(SLOW_VIDEOFEED_H_) */