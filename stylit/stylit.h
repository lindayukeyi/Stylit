#pragma once
#include "pyramid.h"
#include "image.h"
#include <memory>

class Stylit {
public:
	Stylit() {}
	~Stylit(){}

	Stylit(float neighbor);
	Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b, float neighbor, float miu);
	cv::Mat synthesize();
	void averageColor(const cv::Mat* sourceStyle, cv::Mat* targetStyle, int x_p, int y_p, int widthOfSource, int heightOfSource, cv::Vec3f& sourceRGBAvg);

	void setA(std::unique_ptr<Pyramid> a);
	void setAP(std::unique_ptr<Pyramid> ap);
	void setB(std::unique_ptr<Pyramid> b);

	void setNeigbor(float neigh);
	void setMIU(float miu);

	void initialize();

	void setTmpPath(string& str) { this->tmpPath = str; }
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
	float searchWithUniformPatch(int level);
	//void averageColor(const cv::Mat* sourceStyle, cv::Mat* targetStyle, int x_p, int y_p, int widthOfSource, int heightOfSource, cv::Vec3f& sourceRGBAvg);
	float miu; // weight miu
	float neighborSize;
	int bound;
	float energy(float x_s, float y_s, float x_t, float y_t);
	string tmpPath;
};
