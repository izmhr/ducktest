#include "ofMain.h"
#include "ofApp.h"


//========================================================================
int main( ){
	ofGLWindowSettings settings;
	settings.setGLVersion(3, 0);// <-------- setup the GL context
	
	ofCreateWindow(settings);
	ofSetWindowShape(1280, 1000);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
	

}
