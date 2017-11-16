#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	// setup realsense camera
	realsense.setup(true, true);
	realsense.open();            //open the camera
	//set up yolo
	yolo.setup();
	probability = 0.0;           //when the result's prob larger than this ,the box shows;  
	//set up and open OculusRiftCv1
	bool bOK = cv1.init();
	if (!bOK) {
		ofLogNotice() << "OculusRiftCV1 failed to initialize";
	}
	else {
		ofLogNotice() << "Initialized OculusRiftCV1";
		bounds = cv1.getHMDSize();
	}
	//set up ovrpro
	ovrPro.init();
	ovrw = ovrPro.ovr_camWidth;
	ovrh = ovrPro.ovr_camHeight;
	ovrwt = ovrw / 1000;
	ovrht = ovrh / 1000;
	//set parameter
	xp.set("X", 0.45, 0, 10.0); xdp.set("+X", -0.05, -1.0, 1.0);
	yp.set("Y", 0.6, 0, 10.0);	ydp.set("+Y", 0.12, -1.0, 1.0);
	zp.set("Z", 1130, 0, 2000); zdp.set("+Z", 0.53, 0, 2.0);
	sofa.set("sofa", 0.2, 0, 0.5);
	sizep.set("textsize", 100000, 100000, 1000000); ztexture.set("OvrPosOnZ", 0.86, 0, 4);
	gap.set("OvrGapX", 0.048, -0.2, 0.2); gapy.set("OvrGapY", 0.004, -0.2, 0.2);
	gui.setup(); gui.add(xp); gui.add(xdp); gui.add(yp); gui.add(ydp); gui.add(zp);
	gui.add(zdp); gui.add(sizep); gui.add(ztexture); gui.add(gap); gui.add(gapy); gui.add(sofa);
	//load image file
	for (int i = 0; i < 22; i++) {
		obj[i].loadImage(ofToString(i) + ".png");
	}
	//allocate
	colorFromRealsense.allocate(640, 480, OF_PIXELS_RGB);
	//set up dir
	up.set(0.f, 1.f, 0.f);
}

//--------------------------------------------------------------
void ofApp::update() {
	updateAlpha();
	//update for realsense camera
	realsense.update();
	colorFromRealsense = realsense.getColorPixelsRef();

	//depthInColor.loadData(realsense.getDepthPixelsInColorFrame());
	//realsensergb.loadData(colorFromRealsense);            

	//recive detection results from yolo
	lastResults = yolo.detect(colorFromRealsense);
	//lastResults = yolo.tracking(lastResults);                   //add tracking option(optional)
	planes = getPositionOfObj();//caculate the depth for objects

	//update OculusCv1 for posistion and orientation?	
	cv1.update();

	//update ovr
	ovrPro.update();

	//billboarding
	billboarding(position, axis);

	//set 2d position for ovr texture 
	tlx = (axis.x * -ovrwt + axis.y * ovrht);
	tly = (axis.x * ovrwt + axis.y *  ovrht);
	brx = (axis.x * ovrwt + axis.y * -ovrht);
	bry = (axis.x * -ovrwt + axis.y * -ovrht);
	tlx1 = (axis.x * (-ovrwt + gap) + axis.y * (ovrht + gapy));
	tly1 = (axis.x * (ovrwt + gap) + axis.y * (ovrht + gapy));
	brx1 = (axis.x * (ovrwt + gap) + axis.y * (-ovrht + gapy));
	bry1 = (axis.x * (-ovrwt + gap) + axis.y * (-ovrht + gapy));
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(200);
	//draw color image on top left
	realsense.drawColor(0, 0);
	//draw calabrated depth image on top right
	// if (depthInColor.isAllocated()) {
	//  	depthInColor.draw(640, 0);}

	//draw names and boxs of obj on screen depending on the probability;
	drawNameAndBoxFromYolo();

	// draw left eye first
	cv1.begin(ovrEye_Left);
	ofClear(0, 0, 0);
	drawSceneLeft();
	cv1.end(ovrEye_Left);

	// then right eye
	cv1.begin(ovrEye_Right);
	ofClear(0, 0, 0);
	drawSceneRight();
	cv1.end(ovrEye_Right);

	//display the stereo view in the OF window (optional)
	cv1.draw(0, 480, bounds.width, bounds.height);

	//draw gui
	gui.setPosition(bounds.width, 480);
	gui.draw();
}

void ofApp::drawSceneLeft() {
	ofPushMatrix;
	ofTranslate(position);
	ofSetColor(alphaCam, alphaCam, alphaCam);
	drawovrvisionsceneleft();
	ofSetColor(255, 255, 255);
	ofFill();
	drawplane();
	ofPopMatrix;
}

void ofApp::drawSceneRight() {
	ofPushMatrix;
	ofTranslate(position);
	ofSetColor(alphaCam, alphaCam, alphaCam);
	drawovrvisionsceneright();
	ofSetColor(255, 255, 255);
	ofFill();
	drawplane();
	ofPopMatrix;
}

void ofApp::exit() {
	colorFromRealsense.clear();
	realsense.close();
}

void ofApp::drawNameAndBoxFromYolo() {
	for (auto& r : lastResults) {
		if (r.prob >= probability) {
			ofNoFill();
			ofDrawBitmapString(yolo.getName(r.obj_id) + " - " + ofToString(r.prob), r.x, r.y);
			ofDrawRectangle(r.x, r.y, r.w, r.h);
		}
	}
}

//still have a lot of to do with this :(     
std::vector<posAndSize_> ofApp::getPositionOfObj() {
	std::vector<posAndSize_> planes;
	for (auto& r : lastResults) {
		if (r.prob >= probability) {
			float xcenter = r.x + r.w / 2; // get center position of the box;
			float ycenter = r.y + r.h / 2;
			float everydis = 0.0;
			float number = 0.0;
			for (int x_ = r.x + r.w / 4; x_ < r.x + 3 * r.w / 4; x_++) {
				for (int y_ = r.y + r.h / 4; y_ < r.y + 3 * r.h / 4; y_++) {
					float dis_ = realsense.getDistanceAt(x_, y_);
					if (100 < dis_ && dis_ < 9000) {
						float dis = realsense.getDistanceAt(x_, y_);
						everydis += dis;
						number++;
					}
				}
			}
			float avragedis = everydis / number;
			posAndSize_ plane;
			//plane.dis = avragedis;
			plane.xpos = (xcenter - 320.0f) / 320.0f * xp + xdp;     //trying to calabrite with the real word but so far really cranky :(
			plane.ypos = (240.0f - ycenter) / 240.0f * yp + ydp;
			plane.zpos = avragedis / zp + zdp;

			plane.sizeHeight = r.h;
			plane.sizeWidth = r.w;
			plane.number = r.obj_id;

			plane.size = sqrt(plane.sizeWidth * plane.sizeHeight);
			ofVec3f objpos = axis.x * plane.xpos + axis.y * plane.ypos + axis.z * plane.zpos;
			plane.plane.setPosition(objpos.x, objpos.y, objpos.z);
			plane.plane.setResolution(2, 2);
			//plane.lookAt(-axis.z_axis, axis.y_axis);
			plane.plane.lookAt(-axis.z, -up);

			planes.push_back(plane);

			ofLog(OF_LOG_NOTICE, yolo.getName(r.obj_id) + ":" + "x:" + ofToString(xcenter) + "y:"
				+ ofToString(ycenter) + "distance:" + ofToString(avragedis));     //show the obj's position on screen
		}
	}
	//sort(planes.begin(), planes.end());
	return planes;
}


void ofApp::billboarding(ofVec3f & position, axis_ & axis) {
	cv1.getHMDTrackingState(position, orientation);
	Node.setGlobalPosition(position);
	Node.setGlobalOrientation(orientation);       //omg ofnode is amazing!;	
	axis.x = Node.getSideDir();                   //set xyz axis
	axis.y = Node.getUpDir();
	axis.z = Node.getLookAtDir();
}

void ofApp::drawplane() {
	//ofEnableDepthTest();
	ofSetColor(255, 255, 255, alphaText);
	for (auto& i : planes) {
		obj[i.number].getTextureReference().bind();
		i.plane.mapTexCoordsFromTexture(obj[i.number].getTexture());
		i.plane.resizeToTexture(obj[i.number].getTexture(), i.size / sizep);
		i.plane.draw();
		obj[i.number].getTextureReference().unbind();
	}
	ofSetColor(255, 255, 255, 255);
	//ofDisableDepthTest();
}


void ofApp::drawovrvisionsceneleft() {
	ofTranslate(axis.z * ztexture);
	ovrPro.texL.draw(tlx, tly, brx, bry);
	ofTranslate(axis.z * -ztexture);
}

void ofApp::drawovrvisionsceneright() {
	ofTranslate(axis.z * ztexture);
	ovrPro.texR.draw(tlx1, tly1, brx1, bry1);
	ofTranslate(axis.z * -ztexture);
}

void ofApp::updateAlpha()
{
	if (isCamVisible)
	{
		alphavalue += 0.03f;
		if (alphavalue >= 1.0f)
			alphavalue = 1.0f;
	}
	else
	{
		alphavalue -= 0.03f;
		if (alphavalue <= 0.0f)
			alphavalue = 0.0f;
	}
	if (isTextVisible)
	{
		alphavalue_ += 0.03f;
		if (alphavalue_ >= 1.0f)
			alphavalue_ = 1.0f;
	}
	else
	{
		alphavalue_ -= 0.03f;
		if (alphavalue_ <= 0.0f)
			alphavalue_ = 0.0f;
	}
	alphaCam = int(255.0f * alphavalue);
	alphaText = int(255.0f * alphavalue_);
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	if (key == 'a')
	{
		isCamVisible = false;
		isTextVisible = true;
	}
	if (key == 's')
	{
		isCamVisible = true;
		isTextVisible = false;
	}
	if (key == 'd')
	{
		isCamVisible = true;
		isTextVisible = true;
	}
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}