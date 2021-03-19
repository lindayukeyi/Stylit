#pragma once

#include "FeatureVector.h"
#include <vector>
#include <opencv2/opencv.hpp>
class Pyramid {
public:
	int levels;
	std::vector<unique_ptr<FeatureVector>> featureAtAllLevels;

	Pyramid(unique_ptr<FeatureVector> fvec, int levels);
	~Pyramid() {}

	void setVecLevel(unique_ptr<FeatureVector>& fvec, int l);
};
