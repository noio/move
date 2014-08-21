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

    bool processFrame()
    {
        if (!frame_is_new)
        {
            return false;
        }
        lock();
        frame_im.setFromPixels(pixels);
        unlock();
        if (!frame_im.isAllocated())
        {
            frame = cv::Mat::zeros(width, height, CV_8UC3);
            return false;
        }
        int capture_width = frame_im.getWidth();
        int capture_height = frame_im.getHeight();
        float ratio = (float)width / height;
        int w = min(capture_width, static_cast<int>(round(capture_height * ratio)));
        int h = min(capture_height, static_cast<int>(round(capture_width / ratio)));
        roi = cv::Rect((capture_width - w) / 2, (capture_height - h) / 2, w, h);
        frame_mat = toCv(frame_im);
        if (flip < 2)
        {
            cv::flip(frame_mat(roi), frame, flip);
        }
        else
        {
            frame = frame_mat(roi).clone();
        }
        frame_is_new = false;
        return true;
    }

    bool getFrame(Mat& output_frame)
    {
        bool was_new = processFrame();
        output_frame = frame;
        return was_new;
    }

    void draw(float x, float y, float w, float h)
    {
        processFrame();
        if (frame_im.isAllocated())
        {
            if (flip == 1)
            {
                frame_im.drawSubsection(w, 0, -w, h, roi.x, roi.y, roi.width, roi.height);
            }
            else if (flip == 0)
            {
                frame_im.drawSubsection(0, h, w, -h, roi.x, roi.y, roi.width, roi.height);
            }
            else if (flip == -1)
            {
                frame_im.drawSubsection(w, h, -w, -h, roi.x, roi.y, roi.width, roi.height);
            }
            else
            {
                frame_im.drawSubsection(0, 0, w, h, roi.x, roi.y, roi.width, roi.height);
            }
        }
    }

protected:
    int width, height, flip = 1;
    bool frame_is_new = false;
    Mat frame_mat;
    Mat frame;
    ofImage frame_im;
    ofPixels pixels;
    cv::Rect roi;
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