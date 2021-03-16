#pragma once
#include "pyramid.h"
#include "image.h"
#include <memory>

class Stylit {
public:
	Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b);
	image synthesize();

private:
	std::unique_ptr<Pyramid> a;
	std::unique_ptr<Pyramid> ap;
	std::unique_ptr<Pyramid> b;
	std::unique_ptr<Pyramid> bp;

	float NNF(int level);
	void averageColor(int level);
};
