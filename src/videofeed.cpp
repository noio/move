#include "videofeed.h"

void VideoFeedStatic::setup(string path)
{
    frame_im.loadImage(path);
    frame_mat = toCv(frame_im);
}


void VideoFeedWebcam::setup(int device, int capture_width, int capture_height)
{
    Poco::ScopedLock<ofMutex> lock(mutex);
    if (camera.isInitialized())
    {
        camera.close();
    }
    camera.setDeviceID(device);
    camera.initGrabber(capture_width, capture_height, false);
    camera.setUseTexture(false);
    frame_im.setUseTexture(false);
    startThread(true, false);
}

void VideoFeedWebcam::threadedFunction()
{
    while (isThreadRunning())
    {
        if (camera.isInitialized())
        {
            camera.update();
            if (camera.isFrameNew())
            {
                lock();
                frame_im.setFromPixels(camera.getPixelsRef());
                frame_is_new = true;
                unlock();
            }
        }
    }
}

