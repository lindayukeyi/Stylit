#pragma once

#include "image.h"
#include <memory>
#include <opencv2/opencv.hpp>

class FeatureVector {
public:
	std::unique_ptr<cv::Mat> RGB; 
	std::unique_ptr<cv::Mat> LDE;
	std::unique_ptr<cv::Mat> LSE;
	std::unique_ptr<cv::Mat> LDDE;
	std::unique_ptr<cv::Mat> LD12E;

	int level;

	FeatureVector(std::unique_ptr<cv::Mat> rgb, 
					std::unique_ptr<cv::Mat> lde, 
					std::unique_ptr<cv::Mat>lse, 
					std::unique_ptr<cv::Mat> ldde, 
					std::unique_ptr<cv::Mat> ld12e);
	FeatureVector(std::unique_ptr<cv::Mat> rgb,
		std::unique_ptr<cv::Mat> lde,
		std::unique_ptr<cv::Mat>lse,
		std::unique_ptr<cv::Mat> ldde,
		std::unique_ptr<cv::Mat> ld12e,
		int level);

	~FeatureVector() {}

};

