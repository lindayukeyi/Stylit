#include "stylit.h"

#define PatchSize 5
#define EPSILON 0.01
#define MAXCOUNT 6

Stylit::Stylit(std::unique_ptr<Pyramid>& a, std::unique_ptr<Pyramid>& ap, std::unique_ptr<Pyramid>& b, float neighbor)
	: neighborSize(neighbor), bound(neighbor / 2)
{
	a = std::move(a);
	ap = std::move(ap);
	b = std::move(b);

	int levels = b->levels;
	int bCols = b->featureAtAllLevels[levels - 1]->RGB->cols;
	int bRows = b->featureAtAllLevels[levels - 1]->RGB->rows;

	// Initialize bp pyramid
	cv::Mat bpMat(bRows, bCols, CV_8UC1);
	unique_ptr<cv::Mat> bpMatPtr = make_unique<cv::Mat>(bpMat);
	std::unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
	unique_ptr<FeatureVector> bpFvPtr = make_unique<FeatureVector>(bpMatPtr, lde, lse, ldde, ld12e, 0);
	bp = make_unique<Pyramid>(bpFvPtr, levels);

	// Initialize level0 bp
	cv::Mat* sourceStyle = ap->featureAtAllLevels[0].get()->RGB.get();
	cv::Mat* targetStyle = bp->featureAtAllLevels[0].get()->RGB.get();
	int aCols = sourceStyle->cols;
	int aRows = sourceStyle->rows;
	bCols = targetStyle->cols;
	bRows = targetStyle->rows;
	int scales = max(1, bCols * bRows / aCols / aCols);

	std::vector<int> neighborFields = std::vector<int>(bCols * bRows, 0);
	for (int i = 0; i < bCols * bRows; i++)
	{
		neighborFields[i] = i / scales;
	}
	std::random_shuffle(neighborFields.begin(), neighborFields.end());

	for (size_t x_q = 0; x_q < bRows; ++x_q) {
		for (size_t y_q = 0; y_q < bCols; ++y_q) {
			int currPixel = x_q * bCols + y_q;
			int x_p = neighborFields[currPixel] / aCols;
			int y_p = neighborFields[currPixel] % aCols;
			cv::Vec3f sourceRGBAvg(0.0);
			averageColor(sourceStyle, targetStyle, x_p, y_p, aCols, aRows, sourceRGBAvg);
			targetStyle->at<cv::Vec3f>(x_q, y_q) = sourceRGBAvg;
		}
	}
}

float length(const cv::Vec3f& v1, const cv::Vec3f& v2) {
	return (v1[0] - v2[0]) * (v1[0] - v2[0]) + (v1[1] - v2[1]) * (v1[1] - v2[1]) + (v1[2] - v2[2]) * (v1[2] - v2[2]);
}

Stylit::Stylit(float neighbor) : neighborSize(neighbor), bound(neighbor / 2)
{}


cv::Mat Stylit::synthesize()
{
	// EM iteration
	int levels = b->levels;
	int iter = 0, count = 0;
	float preEnergy = FLT_MAX;
	while (iter < levels)
	{
		count++;
		float energy = search(iter);
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
	cv::Mat result = *bp->featureAtAllLevels[levels - 1]->RGB;
	cv::imshow("Stylit", result);
	return result;
}

// Return the whole image's energy
float Stylit::search(int level)
{
	float minErr = 0.0f;
	FeatureVector* sourceObject = a->featureAtAllLevels[level].get();
	cv::Mat* sourceStyle = ap->featureAtAllLevels[level].get()->RGB.get();
	FeatureVector* targetObject = b->featureAtAllLevels[level].get();
	cv::Mat* targetStyle = bp->featureAtAllLevels[level].get()->RGB.get();
	cv::Mat targetStyle_new = targetStyle->clone();

	int widthOfSource = sourceStyle->cols;
	int heightOfSource = sourceStyle->rows;
	int widthOfTarget = targetStyle->cols;
	int heightOfTarget = targetStyle->rows;

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

	// Iterate each pixel in B'
	for (size_t x_q = 0; x_q < heightOfTarget; ++x_q) {
		for (size_t y_q = 0; y_q < widthOfTarget; ++y_q) {
			// Iterate each guidance
			float energy = 0.0f;
			float minEnergy = 0.0f;
			cv::Vec3f sourceRGBAvg(0.0);
			cv::Point2f minP;

			// Iterate each pixel p in A
			for (size_t x_p = 0; x_p < heightOfSource; ++x_p) {
				for (size_t y_p = 0; y_p < widthOfTarget; ++y_p) {
					// Searching neighbors
					float energyP = 0.0f;
					for (size_t x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
						for (size_t y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
							int x_p_neigh = x_p + x_neigh;
							int y_p_neigh = y_p + y_neigh;
							int x_q_neigh = x_q + x_neigh;
							int y_q_neigh = y_q + y_neigh;

							if (x_p_neigh < 0) x_p_neigh = 0;
							if (x_p_neigh >= heightOfSource) x_p_neigh = heightOfSource - 1;
							if (y_p_neigh < 0) y_p_neigh = 0;
							if (y_p_neigh >= widthOfSource) y_p_neigh = widthOfSource - 1;
							if (x_q_neigh < 0) x_q_neigh = 0;
							if (x_q_neigh >= heightOfTarget) x_q_neigh = heightOfTarget - 1;
							if (y_q_neigh < 0) y_q_neigh = 0;
							if (y_q_neigh >= widthOfTarget) y_q_neigh = widthOfTarget - 1;

							energy += NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
								rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
								sourceStyle, targetStyle,
								x_p_neigh, y_p_neigh, x_q_neigh, y_q_neigh);
						}
					}

					// Select the minimum error
					if (energyP < minEnergy) {
						minEnergy = energyP;
						minP = cv::Point2f(x_p, y_p);
					}
				}
			}
			// Averge Color for each pixel q
			averageColor(sourceStyle, targetStyle, minP.x, minP.y, widthOfSource, heightOfSource, sourceRGBAvg);
			targetStyle_new.at<cv::Vec3f>(x_q, y_q) = (targetStyle->at<cv::Vec3f>(x_q, y_q) + sourceRGBAvg) / 2.0f;
			minErr += minEnergy;
		}
	}
	*targetStyle = targetStyle_new.clone();
	return minErr;
}

// Return the energy of a pixel q in image B'
float Stylit::NNF(const cv::Mat* rgbSource, const cv::Mat* ld12eSource, const cv::Mat* lddeSource, const cv::Mat* ldeSource, const cv::Mat* lseSource,
				  const cv::Mat* rgbTarget, const cv::Mat* ld12eTarget, const cv::Mat* lddeTarget, const cv::Mat* ldeTarget, const cv::Mat* lseTarget,
				  const cv::Mat* sourceStyle, const cv::Mat* targetStyle,
				  int x_s, int y_s, int x_t, int y_t)
{
	float guidanceEnergy = length(rgbSource->at<cv::Vec3f>(x_s, y_s), rgbTarget->at<cv::Vec3f>(x_t, y_t)) +
							length(ld12eSource->at<cv::Vec3f>(x_s, y_s), ld12eTarget->at<cv::Vec3f>(x_t, y_t)) +
							length(lddeSource->at<cv::Vec3f>(x_s, y_s), lddeTarget->at<cv::Vec3f>(x_t, y_t)) +
							length(ldeSource->at<cv::Vec3f>(x_s, y_s), ldeTarget->at<cv::Vec3f>(x_t, y_t)) +
							length(lseSource->at<cv::Vec3f>(x_s, y_s), lseTarget->at<cv::Vec3f>(x_t, y_t));
	return length(sourceStyle->at<cv::Vec3f>(x_s, y_s), targetStyle->at<cv::Vec3f>(x_t, y_t)) + miu * guidanceEnergy;
}

void Stylit::averageColor(const cv::Mat* sourceStyle, cv::Mat* targetStyle, int x_p, int y_p, int widthOfSource, int heightOfSource, cv::Vec3f &sourceRGBAvg)
{
	sourceRGBAvg = cv::Vec3f(0.0f);
	float lower = -1 * bound;
	float higer = bound;
	for (int x_neigh = lower; x_neigh <= higer; ++x_neigh) {
		for (int y_neigh = lower; y_neigh <= higer; ++y_neigh) {
			//printf("x: %d, y: %d\n", x_neigh, y_neigh);

			int x_p_neigh = x_p + x_neigh;
			int y_p_neigh = y_p + y_neigh;

			if (x_p_neigh < 0) x_p_neigh = 0;
			if (x_p_neigh >= heightOfSource) x_p_neigh = heightOfSource - 1;
			if (y_p_neigh < 0) y_p_neigh = 0;
			if (y_p_neigh >= widthOfSource) y_p_neigh = widthOfSource - 1;
			printf("x1: %d, y1: %d\n", x_p_neigh, y_p_neigh);

			int c = sourceStyle->channels();
			sourceRGBAvg += sourceStyle->at<cv::Vec3f>(x_p_neigh, y_p_neigh);

		}
	}
	sourceRGBAvg /= float(neighborSize * neighborSize);
	//printf("sourceRGBAvg: %f %f %f\n\n", sourceRGBAvg[0], sourceRGBAvg[1], sourceRGBAvg[2]);

}
