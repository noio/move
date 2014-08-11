#include "ofApp.h"
#include "ofxCv.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    flowcam_here.setup(capture_width, capture_height, ofGetWidth(), ofGetHeight(), 0, 1.0);
    flowcam_there.setup(capture_width, capture_height, ofGetWidth(), ofGetHeight(), 1, 1.0);
    screen.setup(flowcam_there.getScreenTexture(), flowcam_here.getFlowHighTexture());
}

//--------------------------------------------------------------
void ofApp::update()
{
    delta_t = ofGetLastFrameTime();

    flowcam_here.update();
    flowcam_there.update();




    const vector<ofPolyline>& contours = flowcam_here.getContoursHigh();
    for (int ic = 0; ic < contours.size(); ic++)
    {
        Trailshape shape = {0.0, contours[ic].getResampledBySpacing(5.0)};
        trailshapes.push_back(shape);
    }

    for (int is = 0; is < trailshapes.size(); is ++)
    {
        trailshapes[is].t += delta_t;
        trailshapes[is].shape = trailshapes[is].shape.getSmoothed(4);
    }

    while (trailshapes.size() && trailshapes[0].t > 2.0f)
    {
        trailshapes.pop_front();
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{

    ofClear(0);

//    ofSetColor(255, 0, 0, 0);
    glColorMask(true, true, true, false);
    flowcam_here.draw(0,0, ofGetWidth(), ofGetHeight());
//    ofClearAlpha();
//
    glEnable(GL_BLEND);
    glColorMask(false, false, false, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1,1,1,1.0f);

    float scale_flow_to_game = ofGetWidth() / (float)flowcam_here.getFlowSize().width;
    ofPushMatrix();
    ofScale(scale_flow_to_game, scale_flow_to_game);

    for (int ic = 0; ic < trailshapes.size(); ic++)
    {
        ofPolyline poly = trailshapes[ic].shape;
        ofSetColor(255,255,255,255 * (1 - trailshapes[ic].t / 2.0f) * trailshapes[ic].t * 5);
        ofBeginShape();
        vector<ofPoint>& vertices = poly.getVertices();
        for(int j = 0; j < vertices.size(); j++)
        {
            ofVertex(vertices[j]);
        }
        ofEndShape();
    }

    ofPopMatrix();


//    glColorMask(true, true, true, false);
//    ofPushMatrix();
//    ofScale(scale_flow_to_game, scale_flow_to_game);
//    for (int ic = 0; ic < trailshapes.size(); ic++) {
//        ofPolyline poly = trailshapes[ic].shape;
//        poly.draw();
//    }
//    ofPopMatrix();



//    glColorMask(true,true,true,true);
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
//    glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

//    flowcam.getScreenTexture().draw(0, 0);

//    flowcam_here.draw(ofGetWidth() / 2 , 0, ofGetWidth() / 2 , ofGetHeight());
//    flowcam_there.draw(0, 0, ofGetWidth() / 2, ofGetHeight());
//
//    glColorMask(false, false, false, true);
//    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
//    glColor4f(1,1,1,0.5f);
//    ofRect(0, 0, ofGetWidth(), ofGetHeight());

    glColorMask(true, true, true, true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    flowcam_there.draw(0, 0, ofGetWidth(), ofGetHeight());
//    screen.draw(flowcam_there.getScreenTexture(), flowcam_here.getFlowHighTexture());

    glDisable(GL_BLEND);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
