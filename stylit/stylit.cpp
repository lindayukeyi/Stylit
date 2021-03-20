#include "stylit.h"

float length(const cv::Vec3f& v1, const cv::Vec3f& v2) {
	return (v1[0] - v2[0]) * (v1[0] - v2[0]) + (v1[1] - v2[1]) * (v1[1] - v2[1]) + (v1[2] - v2[2]) * (v1[2] - v2[2]);
}

Stylit::Stylit(float neighbor) : neighborSize(neighbor), bound(neighbor / 2)
{}

Stylit::Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b)
{
	// TODO Initialize level1 in bp
	bound = neighborSize / 2;
}

image Stylit::synthesize()
{
	return image();
}

// Return the whole image's energy
float Stylit::search(int level)
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
	for (size_t x_q = 0; x_q < widthOfTarget; ++x_q) {
		for (size_t y_q = 0; y_q < heightOfTarget; ++y_q) {
			// Iterate each guidance
			float energy = 0.0f;
			float minEnergy = 0.0f;
			cv::Vec3f sourceRGBAvg(0.0);
			cv::Point2f minP;

			// Iterate each pixel p in A
			for (size_t x_p = 0; x_p < widthOfSource; ++x_p) {
				for (size_t y_p = 0; y_p < heightOfSource; ++y_p) {
					// Searching neighbors
					float energyP = 0.0f;
					for (size_t x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
						for (size_t y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
							int x_p_neigh = x_p + x_neigh;
							int y_p_neigh = y_p + y_neigh;

							if (x_p_neigh < 0) x_p_neigh = 0;
							if (x_p_neigh >= widthOfSource) x_p_neigh = widthOfSource - 1;
							if (y_p_neigh < 0) y_p_neigh = 0;
							if (y_p_neigh >= heightOfSource) y_p_neigh = heightOfSource - 1;

							energy += NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
								rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
								sourceStyle, targetStyle,
								x_p_neigh, y_p_neigh, x_q, y_q);
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
			targetStyle->at<cv::Vec3f>(x_q, y_q) = (targetStyle->at<cv::Vec3f>(x_q, y_q) + sourceRGBAvg) / 2.0f;
			minErr += minEnergy;
		}
	}
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
			printf("x: %d, y: %d\n", x_neigh, y_neigh);

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
	printf("sourceRGBAvg: %f %f %f\n\n", sourceRGBAvg[0], sourceRGBAvg[1], sourceRGBAvg[2]);

}
