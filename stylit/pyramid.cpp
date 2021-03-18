#include "pyramid.h"

Pyramid::Pyramid(unique_ptr<FeatureVector> fvec, int level)
	:levels(level), featureAtAllLevels(level)
{
	featureAtAllLevels[level - 1] = std::move(fvec);
	for (int i = level - 2; i >= 0; i--)
	{
		FeatureVector* featurePtr = featureAtAllLevels[i + 1].get();

	}
	setVecLevel(fvec, featureAtAllLevels.size());

}

void Pyramid::setVecLevel(unique_ptr<FeatureVector>&fvec, int l)
{
	fvec->level = l;
}