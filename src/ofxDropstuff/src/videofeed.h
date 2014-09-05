#ifndef MOVE_VIDEOFEED_H_
#define MOVE_VIDEOFEED_H_

#include "ofMain.h"
#include "ofxCv.h"

namespace ofxDS
{

    enum VideoFeedWebcamResolution {
        WEBCAM_RES_1080,
        WEBCAM_RES_720,
        WEBCAM_RES_480
    };

    class VideoFeed : public ofThread
    {

    public:
        VideoFeed() {};
        VideoFeed(const VideoFeed&) = delete;            // no copy
        VideoFeed& operator=(const VideoFeed&) = delete; // no assign

        void setAspectRatio(int in_width, int in_height);
        void setFlip(int in_flip);
        void setMaxFps(float in_fps);
        bool processFrame();
        
        /*
         * Puts the latest frame into the given matrix,
         * and returns whether this a fresher frame than
         * the last time you called getMatrix.
         * Therefore, if you depend on the return, use only
         * once per loop.
         */
        bool getFrame(cv::Mat& output_frame);
        void draw(float x, float y, float w, float h);

    protected:
        int width, height;
        char flip = 1;
        float wait_millis = 1000.0f / 30.0f;
        bool needs_processing = false;
        unsigned long long frame_timestamp = 0;
        unsigned long long last_frame_returned = 0;
        cv::Mat frame_mat;
        cv::Mat frame;
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
        ~VideoFeedWebcam() { ofLogVerbose("VideoFeedImageWebcam") << "destroying"; waitForThread(true); }
        void setup(int device, VideoFeedWebcamResolution res) ;
        void threadedFunction() ;
    private:
        ofVideoGrabber camera;
    };

    class VideoFeedImageUrl : public VideoFeed
    {
    public:
        ~VideoFeedImageUrl() { waitForThread(true); }
        void setup(string in_url) ;
        void threadedFunction() ;
    private:
        string url;
        ofImage loader;
        int fail_count;

    };

}

#endif /* defined(MOVE_VIDEOFEED_H_) */