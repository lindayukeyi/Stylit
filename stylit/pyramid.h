#pragma once

#include "FeatureVector.h"
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

class Pyramid {
public:
	int levels;
	std::vector<std::unique_ptr<FeatureVector>> featureAtAllLevels;

	Pyramid(std::unique_ptr<FeatureVector>& fvec, int levels);
	~Pyramid() {}

};
