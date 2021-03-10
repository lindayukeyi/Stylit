#include "FeatureVector.h"

FeatureVector::FeatureVector(std::unique_ptr<image> rgb, std::unique_ptr<image> lde, std::unique_ptr<image> lse, std::unique_ptr<image> ldde, std::unique_ptr<image> ld12e)
{
	RGB = std::move(rgb);
	LDE = std::move(lde);
	LSE = std::move(lse);
	LDDE = std::move(ldde);
	LD12E = std::move(ld12e);
	level = 0;
}

FeatureVector::FeatureVector(std::unique_ptr<image> rgb, std::unique_ptr<image> lde, std::unique_ptr<image> lse, std::unique_ptr<image> ldde, std::unique_ptr<image> ld12e, int level)
{
	RGB = std::move(rgb);
	LDE = std::move(lde);
	LSE = std::move(lse);
	LDDE = std::move(ldde);
	LD12E = std::move(ld12e);
	this->level = level;
}