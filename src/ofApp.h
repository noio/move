#pragma once

#include "flowcam.h"
#include "maskedimage.h"

#include "ofMain.h"

#include <deque>

typedef struct Trailshape {
    double t;
    ofPolyline shape;
} Trailshape;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    FlowCam flowcam_here;
    FlowCam flowcam_there;
    MaskedImage screen;
    
    deque<Trailshape> trailshapes;
    
    float delta_t;

    int capture_width = 1280;
    int capture_height = 720;
		
};
