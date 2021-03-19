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

	float NNF(int level); // Find the sum energy at each level at each iteration
	void averageColor(int level);
	float miu; // weight miu
	float neighborSize;
	float energy(float x_s, float y_s, float x_t, float y_t);
};
