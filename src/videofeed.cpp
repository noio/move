#include "videofeed.h"

void VideoFeedStatic::setup(string path)
{
    frame_im.loadImage(path);
    pixels = frame_im.getPixelsRef();
    frame_is_new = true;
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
                pixels = camera.getPixelsRef();
                frame_is_new = true;
                unlock();
                ofSleepMillis(10);
            }
        }
    }
}

void VideoFeedImageUrl::setup(string new_url)
{
    url = new_url;
    fail_count = 0;
    loader = new ofImage();
    loader->setUseTexture(false);
    startThread(true, false);
}

void VideoFeedImageUrl::threadedFunction()
{
    while (isThreadRunning())
    {
        if (!loader->loadImage(url)){
            ofLogWarning("VideoFeedImageUrl") << "load fail (" << fail_count << ") " << url;
            // When loading fails, the ofImage resets bUseTexture to true
            loader->setUseTexture(false);
            fail_count ++;
            ofSleepMillis(fail_count < 60 ? 10 : 1000);
        } else {
            lock();
            pixels = loader->getPixelsRef();
            unlock();
            frame_is_new = true;
            fail_count = 0;
            ofSleepMillis(10);
        }
        
    }
}