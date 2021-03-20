#pragma once
#include "pyramid.h"
#include "image.h"
#include <memory>

class Stylit {
public:
	Stylit(float neighbor);
	Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b);
	cv::Mat synthesize();
	void averageColor(const cv::Mat* sourceStyle, cv::Mat* targetStyle, int x_p, int y_p, int widthOfSource, int heightOfSource, cv::Vec3f& sourceRGBAvg);


private:
	std::unique_ptr<Pyramid> a;
	std::unique_ptr<Pyramid> ap;
	std::unique_ptr<Pyramid> b;
	std::unique_ptr<Pyramid> bp;

	float NNF(const cv::Mat* rgbSource, const cv::Mat* ld12eSource, const cv::Mat* lddeSource, const cv::Mat* ldeSource, const cv::Mat* lseSource,
				const cv::Mat* rgbTarget, const cv::Mat* ld12eTarget, const cv::Mat* lddeTarget, const cv::Mat* ldeTarget, const cv::Mat* lseTarget,
				const cv::Mat* sourceStyle, const cv::Mat* targetStyle,
				int x_s, int y_s, int x_t, int y_t);
	float search(int level);// Find the sum energy at each level at each iteration
	//void averageColor(const cv::Mat* sourceStyle, cv::Mat* targetStyle, int x_p, int y_p, int widthOfSource, int heightOfSource, cv::Vec3f& sourceRGBAvg);
	float miu; // weight miu
	float neighborSize;
	int bound;
	float energy(float x_s, float y_s, float x_t, float y_t);
};
