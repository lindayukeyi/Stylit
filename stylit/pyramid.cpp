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
		std::unique_ptr<cv::Mat> rgb = make_unique<cv::Mat>(RGBDown);
		std::unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);

		if (featurePtr->LDE != nullptr)
		{
			cv::pyrDown(*featurePtr->LDE, LDEDown, downSize);
			lde = make_unique<cv::Mat>(LDEDown);
		}
		if (featurePtr->LSE != nullptr)
		{
			cv::pyrDown(*featurePtr->LSE, LSEDown, downSize);
			lse = make_unique<cv::Mat>(LSEDown);
		}
		if (featurePtr->LDDE != nullptr)
		{
			cv::pyrDown(*featurePtr->LDDE, LDDEDown, downSize);
			ldde = make_unique<cv::Mat>(LDDEDown);
		}
		if (featurePtr->LD12E != nullptr)
		{
			cv::pyrDown(*featurePtr->LD12E, LD12EDown, downSize);
			ld12e = make_unique<cv::Mat>(LD12EDown);
		}

		unique_ptr<FeatureVector> featureDownPtr = make_unique<FeatureVector>(rgb, lde, lse, ldde, ld12e, i);
		featureAtAllLevels[i] = std::move(featureDownPtr);
	}
}

