#pragma once
#include "pyramid.h"
#include "image.h"
#include <memory>

class Stylit {
public:
	Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b);
	cv::Mat synthesize();

private:
	std::unique_ptr<Pyramid> a;
	std::unique_ptr<Pyramid> ap;
	std::unique_ptr<Pyramid> b;
	std::unique_ptr<Pyramid> bp;

	float NNF(int level);
	std::vector<std::vector<int>> neighborFields;
	void averageColor(int level);
};
