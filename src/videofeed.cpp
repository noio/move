#include "videofeed.h"

using namespace ofxCv;

void VideoFeedStatic::setup(string path)
{
    image.loadImage(path);
    frame_full = toCv(image);
}

void VideoFeedStatic::setAspectRatio(int width, int height)
{
    
}

bool VideoFeedStatic::getFrame(Mat& frame)
{
    frame = toCv(image);
    return true;
}

