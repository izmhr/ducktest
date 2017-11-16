#pragma once

#include "ofMain.h"
#include "ofxYolo2.h"
#include "ofxRealSense.h"
#include "ofxOculusRiftCV1.h"
#include "ofxGui.h"
#include "OvrPro.h"

//to restore and position and size data of those objects which will show at OculusCv1
struct posAndSize_
{
	float xpos;
	float ypos;
	float zpos;
	float sizeWidth;
	float sizeHeight;
	int number;
	ofPlanePrimitive plane;
	float size;
	//float dis;

	/*bool operator<(const posAndSize_& another) const {
		return dis > another.dis;
	}*/
};

class ofApp : public ofBaseApp {

private:
	// basic things
	ofxRealSense realsense;
	ofxOculusRiftCV1 cv1;
	OvrPro ovrPro;
	ofxYolo2 yolo;

	// REALSENSE buffers
	ofPixels colorFromRealsense;
	ofTexture depthInColor/*, colorInDepth, */;

	// about HMD posture and position.
	ofVec3f up;
	ofVec3f position;
	ofQuaternion orientation;
	struct axis_
	{
		ofVec3f x;
		ofVec3f y;
		ofVec3f z;
	};
	axis_ axis;
	ofNode Node;

	// using results, draw texts.
	float probability = 0.0f;	//when the result's prob larger than this ,the box shows;  
	std::vector<bbox_t> lastResults;
	std::vector<posAndSize_> planes;
	ofImage textImage[23];

	// GUI THINGS
	ofParameter<float> xp;
	ofParameter<float> yp;
	ofParameter<float> zp;
	ofParameter<float> zdp;
	ofParameter<int>  sizep;
	ofParameter<float> xdp;
	ofParameter<float> ydp;
	ofParameter<float> ztexture;
	ofParameter<float> gap;
	ofParameter<float> gapy;
	ofParameter<float> sofa;
	ofxPanel gui;

	// parameters for CV1 draw
	ofRectangle cv1DrawBounds;

	// parameters for OVRVISION draw
	float ovrw;
	float ovrh;
	float ovrwt;
	float ovrht;
	ofPoint tlx_l;
	ofPoint tly_l;
	ofPoint brx_l;
	ofPoint bry_l;
	ofPoint tlx_r;
	ofPoint tly_r;
	ofPoint brx_r;
	ofPoint bry_r;

	// for scene transition
	float alphavalue = 1.0f;
	float alphavalue_ = 0.0f;
	int alphaCam = 255;
	int alphaText = 0;
	bool isCamVisible = true;
	bool isTextVisible = false;

	// -------------------------------------
	// public FUNCTIONS
	std::vector<posAndSize_> getPositionOfObj();
	void drawNameAndBoxFromYolo();     //draw box on screen, the range of prob is from 0 to 1;
	void billboarding(ofVec3f & position, axis_ & axis);
	void drawPlane();
	void drawOvrvisionSceneLeft();
	void drawOvrvisionSceneRight();
	void updateAlpha();

public:
	void setup();
	void update();
	void draw();
	void exit();
	void drawSceneLeft();
	void drawSceneRight();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
};
