#include "FeatureVector.h"

FeatureVector::FeatureVector(std::unique_ptr<cv::Mat> rgb, std::unique_ptr<cv::Mat> lde, std::unique_ptr<cv::Mat> lse, std::unique_ptr<cv::Mat> ldde, std::unique_ptr<cv::Mat> ld12e)
{
	RGB = std::move(rgb);
	LDE = std::move(lde);
	LSE = std::move(lse);
	LDDE = std::move(ldde);
	LD12E = std::move(ld12e);
	level = 0;
}

FeatureVector::FeatureVector(std::unique_ptr<cv::Mat> rgb, std::unique_ptr<cv::Mat> lde, std::unique_ptr<cv::Mat> lse, std::unique_ptr<cv::Mat> ldde, std::unique_ptr<cv::Mat> ld12e, int level)
{
	RGB = std::move(rgb);
	LDE = std::move(lde);
	LSE = std::move(lse);
	LDDE = std::move(ldde);
	LD12E = std::move(ld12e);
	this->level = level;
}