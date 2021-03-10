#pragma once

#include "FeatureVector.h"
#include <vector>

class Pyramid {
public:
	int levels;

	std::vector<FeatureVector> featureAtAllLevels;

};

