
#ifndef MOVE_MASKEDIMAGE_H_
#define MOVE_MASKEDIMAGE_H_

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxShader.h"

#include <iostream>

class MaskedImage
{
public:
    MaskedImage(){};
    MaskedImage(const MaskedImage&) = delete;            // no copy
    MaskedImage& operator=(const MaskedImage&) = delete; // no assign
    
    void setup(ofImage texture, ofImage mask);
    void draw(ofImage in_texture, ofImage in_mask);
    
private:
    ofShader shader;
    ofImage texture;
    ofImage mask;
    
};


#endif /* defined(MOVE_MASKEDIMAGE_H_) */
