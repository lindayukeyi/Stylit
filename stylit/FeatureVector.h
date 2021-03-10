#pragma once

#include "image.h"
#include <memory>

class FeatureVector {
public:
	std::unique_ptr<image> RGB; 
	std::unique_ptr<image> LDE;
	std::unique_ptr<image> LSE;
	std::unique_ptr<image> LDDE;
	std::unique_ptr<image> LD12E;

	int level;

	FeatureVector(std::unique_ptr<image> rgb, 
					std::unique_ptr<image> lde, 
					std::unique_ptr<image>lse, 
					std::unique_ptr<image> ldde, 
					std::unique_ptr<image> ld12e);
	~FeatureVector() {}

};

