#include "stylit.h"

#define PatchSize 5
#define EPSILON 0.01
#define MAXCOUNT 6

Stylit::Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b)
{

	int levels = b->levels;
	int bCols = b->featureAtAllLevels[levels - 1]->RGB->cols;
	int bRows = b->featureAtAllLevels[levels - 1]->RGB->rows;
	
	// Initialize level0 bp
	cv::Mat bpMat(bRows, bCols, CV_8UC1);
	unique_ptr<cv::Mat> bpMatPtr = make_unique<cv::Mat>(bpMat);
	std::unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
	unique_ptr<FeatureVector> bpFvPtr = make_unique<FeatureVector>(bpMatPtr, lde, lse, ldde, ld12e, 0);
	bp = make_unique<Pyramid>(bpFvPtr, levels);

	// Initialize neighborField
	neighborFields = std::vector<std::vector<int>>(levels);
	for (int i = levels - 1; i >= 0; i--)
	{
		neighborFields[i] = std::vector<int>(bCols * bRows, 0);
		bCols /= 2;
		bRows /= 2;
	}

	int aCols = a->featureAtAllLevels[levels - 1]->RGB->cols;
	int aRows = a->featureAtAllLevels[levels - 1]->RGB->rows;

	int scales = max(1, bCols * bRows / aCols / aCols);

	for (int i = 0; i < bCols * bRows; i++)
	{
		neighborFields[0][i] = i / scales;
	}
	std::random_shuffle(neighborFields[0].begin(), neighborFields[0].end());
	averageColor(0);
}

cv::Mat Stylit::synthesize()
{
	// EM iteration
	int levels = b->levels;
	int iter = 0, count = 0;
	float preEnergy = FLT_MAX;
	while (iter < levels)
	{
		count++;
		float energy = NNF(iter);
		averageColor(iter);
		if (abs(preEnergy - energy) / preEnergy < EPSILON || count > MAXCOUNT) {
			cv::Mat* currMat = bp->featureAtAllLevels[iter]->RGB.get();
			if (iter + 1 < levels)
			{
				cv::Mat* nextMat = bp->featureAtAllLevels[iter + 1]->RGB.get();
				cv::Size upSize = cv::Size(currMat->cols * 2, currMat->rows * 2);
				cv::pyrUp(*currMat, *nextMat, upSize);
			}
			iter++;
			count = 0;
		}
	}
	return *bp->featureAtAllLevels[levels - 1]->RGB;
}

float Stylit::NNF(int level)
{
	return 0.0f;
}

void Stylit::averageColor(int level)
{
}
