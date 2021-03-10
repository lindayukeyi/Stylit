#pragma once

#include "FeatureVector.h"
#include <vector>

class Pyramid {
public:
	int levels;
	std::vector<unique_ptr<FeatureVector>> featureAtAllLevels;

	Pyramid(unique_ptr<FeatureVector> fvec);
	~Pyramid() {}

	void setVecLevel(unique_ptr<FeatureVector>& fvec, int l);
};

