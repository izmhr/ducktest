#pragma once


#include "yolo_v2_class.hpp"	
#include "ofMain.h"



class ofxYolo2 {

	Detector* detector;
	std::vector<std::string> objectNames;

public:

	void setup();

	std::string getName(int index);
	image_t convert(ofPixels & pix);

	std::vector<bbox_t> detect(ofPixels& pixels);
	std::vector<bbox_t> tracking(std::vector<bbox_t> cur_bbox_vec);


};