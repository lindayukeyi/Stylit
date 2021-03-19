#include "stylit.h"

float length(const cv::Vec3f& v1, const cv::Vec3f& v2) {
	return (v1[0] - v2[0]) * (v1[0] - v2[0]) + (v1[1] - v2[1]) * (v1[1] - v2[1]) + (v1[2] - v2[2]) * (v1[2] - v2[2]);
}

Stylit::Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b)
{
	// TODO Initialize level1 in bp
}

image Stylit::synthesize()
{
	return image();
}

float Stylit::NNF(int level)
{
	float minErr = 0.0f;
	FeatureVector* sourceObject = a->featureAtAllLevels[level].get();
	cv::Mat* sourceStyle = ap->featureAtAllLevels[level].get()->RGB.get();
	FeatureVector* targetObject = b->featureAtAllLevels[level].get();
	cv::Mat* targetStyle = bp->featureAtAllLevels[level].get()->RGB.get();

	int widthOfSource = sourceStyle->cols;
	int heightOfSource = sourceStyle->rows;
	int widthOfTarget = targetStyle->cols;
	int heightOfTarget = targetStyle->rows;
	// Iterate each pixel in B'
	for (size_t x_t = 0; x_t < widthOfTarget; ++x_t) {
		for (size_t y_t = 0; y_t < heightOfTarget; ++y_t) {
			// Iterate each guidance
			float energyP = 0.0f;
			float minEnergyP = 0.0f;
			cv::Point2f minQ;
			cv::Mat* rgbSource = sourceObject->RGB.get();
			cv::Mat* ld12eSource = sourceObject->LD12E.get();
			cv::Mat* lddeSource = sourceObject->LDDE.get();
			cv::Mat* ldeSource = sourceObject->LDE.get();
			cv::Mat* lseSource = sourceObject->LSE.get();

			cv::Mat* rgbTarget = targetObject->RGB.get();
			cv::Mat* ld12eTarget = targetObject->LD12E.get();
			cv::Mat* lddeTarget = targetObject->LDDE.get();
			cv::Mat* ldeTarget = targetObject->LDE.get();
			cv::Mat* lseTarget = targetObject->LSE.get();

			float guidanceEnergy = 0.0f;
			// Iterate each pixel p in A
			for (size_t x_s = 0; x_s < widthOfSource; ++x_s) {
				for (size_t y_s = 0; y_s < heightOfSource; ++y_s) {
					// Searching neighbors
					
					// Calculate the energy function
					guidanceEnergy = length(rgbSource->at<cv::Vec3f>(x_s, y_s), rgbTarget->at<cv::Vec3f>(x_t, y_t)) + 
									length(ld12eSource->at<cv::Vec3f>(x_s, y_s), ld12eTarget->at<cv::Vec3f>(x_t, y_t)) +
									length(lddeSource->at<cv::Vec3f>(x_s, y_s), lddeTarget->at<cv::Vec3f>(x_t, y_t)) +
									length(ldeSource->at<cv::Vec3f>(x_s, y_s), ldeTarget->at<cv::Vec3f>(x_t, y_t)) +
									length(lseSource->at<cv::Vec3f>(x_s, y_s), lseTarget->at<cv::Vec3f>(x_t, y_t));
					energyP = length(sourceStyle->at<cv::Vec3f>(x_s, y_s), targetStyle->at<cv::Vec3f>(x_t, y_t)) + miu * guidanceEnergy;
					// Select the minimum error
					if (energyP < minEnergyP) {
						minEnergyP = energyP;
						minQ = cv::Point2f(x_s, y_s);
					}
				}
			}
			// Averge Color for each pixel q

		}
	}
	return minErr;
}

void Stylit::averageColor(int level)
{
}

