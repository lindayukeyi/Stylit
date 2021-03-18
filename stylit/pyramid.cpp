#include "pyramid.h"

Pyramid::Pyramid(unique_ptr<FeatureVector> fvec, int level)
	:levels(level), featureAtAllLevels(level)
{
	featureAtAllLevels[level - 1] = std::move(fvec);
	featureAtAllLevels[level - 1]->level = level - 1;
	for (int i = level - 2; i >= 0; i--)
	{
		FeatureVector* featurePtr = featureAtAllLevels[i + 1].get();
		int cols = featurePtr->RGB->cols / 2;
		int rows = featurePtr->RGB->rows / 2;
		cv::Size downSize = cv::Size(cols, rows);
		cv::Mat RGBDown, LDEDown, LSEDown, LDDEDown, LD12EDown;
		cv::pyrDown(*featurePtr->RGB, RGBDown, downSize);
		cv::pyrDown(*featurePtr->LDE, LDEDown, downSize);
		cv::pyrDown(*featurePtr->LSE, LSEDown, downSize);
		cv::pyrDown(*featurePtr->LDDE, LDDEDown, downSize);
		cv::pyrDown(*featurePtr->LD12E, LD12EDown, downSize);
		std::unique_ptr<cv::Mat> rgb = make_unique<cv::Mat>(RGBDown);
		std::unique_ptr<cv::Mat> lde = make_unique<cv::Mat>(LDEDown);
		std::unique_ptr<cv::Mat> lse = make_unique<cv::Mat>(LSEDown);
		std::unique_ptr<cv::Mat> ldde = make_unique<cv::Mat>(LDDEDown);
		std::unique_ptr<cv::Mat> ld12e = make_unique<cv::Mat>(LD12EDown);

		unique_ptr<FeatureVector> featureDownPtr = make_unique<FeatureVector>(rgb, lde, lse, ldde, ld12e, i);
		featureAtAllLevels[i] = std::move(featureDownPtr);
	}
	

}

void Pyramid::setVecLevel(unique_ptr<FeatureVector>&fvec, int l)
{
	fvec->level = l;
}