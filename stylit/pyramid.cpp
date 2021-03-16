#include "pyramid.h"

Pyramid::Pyramid(unique_ptr<FeatureVector> fvec)
{
	setVecLevel(fvec, featureAtAllLevels.size());
	featureAtAllLevels.push_back(std::move(fvec));
}

void Pyramid::setVecLevel(unique_ptr<FeatureVector>&fvec, int l)
{
	fvec->level = l;
}