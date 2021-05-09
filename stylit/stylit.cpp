#include "stylit.h"
#include <unordered_set>
#include <string>
#include <maya/MGlobal.h>
#include <thread>

#define PatchSize 5
#define EPSILON 0.0001
#define MAXCOUNT 50
#define CORE 3


const float gaussian[5][5] = {
	0.003765,	0.015019,	0.023792,	0.015019,	0.003765,
	0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
	0.023792,	0.094907,	0.150342,	0.094907,	0.023792,
	0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
	0.003765,	0.015019,	0.023792,	0.015019,	0.003765 };
/*
const float gaussian[5][5] = {
	0.04,	0.04,	0.04,	0.04,	0.04,
	0.04,	0.04,	0.04,	0.04,	0.04,
	0.04,	0.04,	0.04,	0.04,	0.04,
	0.04,	0.04,	0.04,	0.04,	0.04,
	0.04,	0.04,	0.04,	0.04,	0.04 };*/
	/*
const float gaussian[3][3] = {
	0.03943601, 0.119713  , 0.03943601 ,
	   0.119713  , 0.36340399, 0.119713,
	   0.03943601, 0.119713  , 0.03943601
};*/

Stylit::Stylit(std::unique_ptr<Pyramid> a, std::unique_ptr<Pyramid> ap, std::unique_ptr<Pyramid> b, float neighbor, float miu)
	: neighborSize(neighbor), bound(neighbor / 2), miu(miu)
{
	printf("Initialization Begin!");
	this->a = std::move(a);
	this->ap = std::move(ap);
	this->b = std::move(b);

	int levels = this->b->levels;
	int bCols = this->b->featureAtAllLevels[levels - 1]->RGB->cols;
	int bRows = this->b->featureAtAllLevels[levels - 1]->RGB->rows;

	// Initialize bp pyramid

	cv::Mat bpMat(bRows, bCols, CV_32FC3, cv::Scalar(0, 0, 0));
	unique_ptr<cv::Mat> bpMatPtr = make_unique<cv::Mat>(bpMat);
	std::unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
	unique_ptr<FeatureVector> bpFvPtr = make_unique<FeatureVector>(bpMatPtr, lde, lse, ldde, ld12e, 0);
	this->bp = make_unique<Pyramid>(bpFvPtr, levels);

	// Initialize level0 bp
	cv::Mat* sourceStyle = this->ap->featureAtAllLevels[0].get()->RGB.get();
	cv::Mat* targetStyle = this->bp->featureAtAllLevels[0].get()->RGB.get();
	int aCols = sourceStyle->cols;
	int aRows = sourceStyle->rows;
	bCols = targetStyle->cols;
	bRows = targetStyle->rows;
	int scales = max(1, bCols * bRows / aCols / aRows);

	std::vector<int> neighborFields = std::vector<int>(bCols * bRows, 0);
	for (int i = 0; i < bCols * bRows; i++)
	{
		neighborFields[i] = i / scales;
	}
	std::random_shuffle(neighborFields.begin(), neighborFields.end());
	patchMatch = neighborFields;

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
	printf("Initialization End!\n");

}


float length(const cv::Vec3f& v1, const cv::Vec3f& v2) {
	return (v1[0] - v2[0]) * (v1[0] - v2[0]) + (v1[1] - v2[1]) * (v1[1] - v2[1]) + (v1[2] - v2[2]) * (v1[2] - v2[2]);
}

Stylit::Stylit(float neighbor) : neighborSize(neighbor), bound(neighbor / 2)
{}


void Stylit::initialize() {
	printf("Initialization Begin!");

	int levels = this->b->levels;
	int bCols = this->b->featureAtAllLevels[levels - 1]->RGB->cols;
	int bRows = this->b->featureAtAllLevels[levels - 1]->RGB->rows;

	// Initialize bp pyramid

	cv::Mat bpMat(bRows, bCols, CV_32FC3, cv::Scalar(0, 0, 0));
	unique_ptr<cv::Mat> bpMatPtr = make_unique<cv::Mat>(bpMat);
	std::unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
	unique_ptr<FeatureVector> bpFvPtr = make_unique<FeatureVector>(bpMatPtr, lde, lse, ldde, ld12e, 0);
	this->bp = make_unique<Pyramid>(bpFvPtr, levels);

	// Initialize level0 bp
	cv::Mat* sourceStyle = this->ap->featureAtAllLevels[0].get()->RGB.get();
	cv::Mat* targetStyle = this->bp->featureAtAllLevels[0].get()->RGB.get();
	int aCols = sourceStyle->cols;
	int aRows = sourceStyle->rows;
	bCols = targetStyle->cols;
	bRows = targetStyle->rows;

	int scales = max(1, bCols * bRows / aCols / aRows);

	std::vector<int> neighborFields = std::vector<int>(bCols * bRows, 0);
	for (int i = 0; i < bCols * bRows; i++)
	{
		neighborFields[i] = i / scales;
	}
	std::random_shuffle(neighborFields.begin(), neighborFields.end());
	patchMatch = neighborFields;


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
	printf("Initialization End!\n");
}


cv::Mat Stylit::synthesize()
{
	printf("Synthesize Begin!\n");
	MGlobal::displayInfo("Synthesize Begin!\n");

	// EM iteration
	int levels = b->levels;
	int iter = 0, count = 0;
	float preEnergy = FLT_MAX;
	while (iter < levels)
	{
		count++;
		//printf("iter:%d    ", iter);
		MGlobal::displayInfo(to_string(iter).c_str());
		cv::Size curSize = bp->featureAtAllLevels[iter]->RGB->size();
		cv::Size curSizeA = ap->featureAtAllLevels[iter]->RGB->size();
		float scale = curSize.height * curSize.width;

		
		int widthStep = curSize.width / CORE;
		int heightStep = curSize.height / CORE;
		std::vector<std::thread> threads;
		std::vector<float> energyVec(CORE * CORE, 0);

		for (int w = 0; w < CORE; ++w) {
			for (int h = 0; h < CORE; ++h) {
				if (h == CORE - 1) {
					threads.push_back(std::thread(&Stylit::searchWithPatchMatch, this, iter, w * widthStep,
						(w + 1) * widthStep,
						h * heightStep,
						curSize.height, std::ref(energyVec[w * CORE + h])));

				}
				else if (w == CORE - 1) {

					threads.push_back(std::thread(&Stylit::searchWithPatchMatch, this, iter, w * widthStep,
						curSize.width,
						h * heightStep,
						(h + 1) * heightStep, std::ref(energyVec[w * CORE + h])));

				}
				else {
					threads.push_back(std::thread(&Stylit::searchWithPatchMatch, this, iter, w * widthStep,
						(w + 1) * widthStep,
						h * heightStep,
						(h + 1) * heightStep, std::ref(energyVec[w * CORE + h])));
				}

			}
		}

		for (int w = 0; w < CORE; ++w) {
			for (int h = 0; h < CORE; ++h) {
				threads[w * CORE + h].join();
			}
		}


		float energy = 0;
		for (int w = 0; w < CORE; ++w) {
			for (int h = 0; h < CORE; ++h) {
				energy += energyVec[w * CORE + h];
			}
		}
		energy /= scale;/*
		//float energy = search(iter);
		printf("energy:%f\n", energy);
		MGlobal::displayInfo(to_string(energy).c_str());
		
		MGlobal::displayInfo(to_string(curSize.width).c_str());
		MGlobal::displayInfo(to_string(curSize.height).c_str());

		float energy = 0;
		searchWithPatchMatch(iter, 0, curSize.width, 0, curSize.height, energy);*/

		if (abs(preEnergy - energy) / preEnergy < EPSILON || count > MAXCOUNT) {
			cv::Mat* currMat = bp->featureAtAllLevels[iter]->RGB.get();
			if (iter + 1 < levels)
			{
				cv::Mat* nextMat = bp->featureAtAllLevels[iter + 1]->RGB.get();
				cv::Mat* nextMatSource = ap->featureAtAllLevels[iter + 1]->RGB.get();

				cv::Size upSize = cv::Size(currMat->cols * 2, currMat->rows * 2);
				this->patchMatch.clear();
				randomAssignPatch(this->patchMatch, nextMatSource->rows, nextMatSource->cols, upSize.height, upSize.width);
				cv::pyrUp(*currMat, *nextMat, upSize);
			}
			iter++;
			count = 0;
			preEnergy = FLT_MAX;
		}
		else {
			preEnergy = energy;
		}
		if (iter < levels) {
			if (count % 10 == 0) {
				cv::Mat tmp = (bp->featureAtAllLevels[iter]->RGB->clone());
				tmp *= 255.0f;
				cv::imwrite(this->tmpPath + "/tmp_" + to_string(iter) + "_" + to_string(count) + ".jpg", tmp);

			}

		}
	}
	printf("Synthesize End!\n");
	cv::Mat result = *(bp->featureAtAllLevels[levels - 1]->RGB);
	result *= 255.0f;
	cv::imwrite(this->tmpPath + "/result.jpg", result);
	cv::imshow("Stylit", result);


	return result;
}

// Return the whole image's energy
void Stylit::search(int level, int startWidth, int endWidth, int startHeight, int endHeight, float &energy)
{
	MGlobal::displayInfo(to_string(startWidth).c_str());
	MGlobal::displayInfo(to_string(endWidth).c_str());
	MGlobal::displayInfo(to_string(startHeight).c_str());
	MGlobal::displayInfo(to_string(endHeight).c_str());
	MGlobal::displayInfo("\n");

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
	for (size_t x_q = startHeight; x_q < endHeight; ++x_q) {
		for (size_t y_q = startWidth; y_q < endWidth; ++y_q) {

			// Iterate each guidance
			float minEnergy = FLT_MAX;
			cv::Vec3f sourceRGBAvg(0.0);
			cv::Point2f minP;

			// Iterate each pixel p in A
			for (size_t x_p = 0; x_p < heightOfSource; ++x_p) {
				for (size_t y_p = 0; y_p < widthOfSource; ++y_p) {
					// Searching neighbors
					float energyP = 0.0f;
					for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
						for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
							float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

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

							energyP += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
								rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
								sourceStyle, targetStyle,
								x_p_neigh, y_p_neigh, x_q_neigh, y_q_neigh);
						}
					}
					//printf("%f\n", energyP);
					//MGlobal::displayInfo(to_string(energyP).c_str());
					// Select the minimum error
					if (energyP < minEnergy) {
						minEnergy = energyP;
						minP = cv::Point2f(x_p, y_p);
					}
				}
			}
			// Averge Color for each pixel q
			averageColor(sourceStyle, targetStyle, minP.x, minP.y, widthOfSource, heightOfSource, sourceRGBAvg);
			targetStyle_new.at<cv::Vec3f>(x_q, y_q) = sourceRGBAvg;
			minErr += minEnergy;

		}
		//printf("x:%d energy:%f\n", x_q, minErr);
		//MGlobal::displayInfo("x:" + x_q);
		//MGlobal::displayInfo("\n");

	}
	for (size_t x_q = startHeight; x_q < endHeight; ++x_q) {
		for (size_t y_q = startWidth; y_q < endWidth; ++y_q) {
			targetStyle->at<cv::Vec3f>(x_q, y_q) = targetStyle_new.at<cv::Vec3f>(x_q, y_q);
		}
	}
	//*targetStyle = targetStyle_new.clone();
	energy =  minErr;
}

// Return the whole image's energy
void Stylit::searchWithPatchMatch(int level, int startWidth, int endWidth, int startHeight, int endHeight, float& energy)
{
		printf("start search iter: %d       ", level);

	float minErr = 0.0f;
	FeatureVector* sourceObject = a->featureAtAllLevels[level].get();
	cv::Mat* sourceStyle = ap->featureAtAllLevels[level].get()->RGB.get();
	FeatureVector* targetObject = b->featureAtAllLevels[level].get();
	cv::Mat* targetStyle = bp->featureAtAllLevels[level].get()->RGB.get();
	cv::Mat targetStyle_new = targetStyle->clone();


	int aCols = sourceStyle->cols;
	int aRows = sourceStyle->rows;
	int bCols = targetStyle->cols;
	int bRows = targetStyle->rows;

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

	int vecsize = this->patchMatch.size() - 1;
	MGlobal::displayInfo(to_string(vecsize).c_str());

	int sourceSize = aCols * aRows;

	// Iterate each pixel in B'
	for (size_t x_q = startHeight; x_q < endHeight; ++x_q) {
		for (size_t y_q = startWidth; y_q < endWidth; ++y_q) {
			//printf("x_q: %d y_q: %d      ", x_q, y_q);
			// Iterate each guidance
			cv::Vec3f sourceRGBAvg(0.0);
			cv::Point2f minP;

			int cur_index = x_q * bCols + y_q;
			int left_index = x_q * bCols + std::max(int(y_q - 1), 0);
			left_index = left_index < 0 ? (rand() % sourceSize) : left_index;

			int right_index = x_q * bCols + std::min(int(y_q + 1), bCols - 1);
			right_index = right_index > vecsize ? (rand() % sourceSize) : right_index;

			int up_index = std::max(0, int(x_q - 1)) * bCols + y_q;
			up_index = up_index < 0 ? (rand() % sourceSize) : up_index;

			int bottom_index = std::min(bRows - 1, int(x_q + 1)) * bCols + y_q;
			bottom_index = bottom_index > vecsize ? (rand() % sourceSize) : bottom_index;

			float new_patch = -1;
			float energyAtEachPixel = 0;

			// Propagate
			
			if (level % 2 == 0) {
				// At even iteration, right and bottom
				new_patch = propagate(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
					rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
					sourceStyle, targetStyle, this->patchMatch[cur_index], this->patchMatch[right_index], this->patchMatch[bottom_index], x_q, y_q, aCols, energyAtEachPixel);
			}
			else {
				// At odd iteration, left and up and cur
				new_patch = propagate(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
					rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
					sourceStyle, targetStyle, this->patchMatch[cur_index], this->patchMatch[left_index], this->patchMatch[up_index], x_q, y_q, aCols, energyAtEachPixel);
			}
			//printf("%f\n", energyAtEachPixel);
			this->patchMatch[cur_index] = new_patch;
			new_patch = randomSearch(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
				rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
				sourceStyle, targetStyle, x_q, y_q, energyAtEachPixel);
			this->patchMatch[cur_index] = new_patch;


			minErr += energyAtEachPixel;


		}

	}
	printf("%f\n", minErr);

	for (size_t x_q = startHeight; x_q < endHeight; ++x_q) {
		for (size_t y_q = startWidth; y_q < endWidth; ++y_q) {
			int source_patch = this->patchMatch[x_q * bCols + y_q];
			int x_p = source_patch / aCols;
			int y_p = source_patch % aCols;
			cv::Vec3f sourceRGBAvg(0.0);
			averageColor(sourceStyle, targetStyle, x_p, y_p, aCols, aRows, sourceRGBAvg);
			targetStyle->at<cv::Vec3f>(x_q, y_q) = sourceRGBAvg;

		}
	}
	energy = minErr;
}

float Stylit::searchWithUniformPatch(int level)
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

	std::unordered_set<std::string> visited;

	float k = 0.0f;

	// Iterate each pixel in B'
	for (size_t x_q = 0; x_q < heightOfTarget; ++x_q) {
		for (size_t y_q = 0; y_q < widthOfTarget; ++y_q) {
			// Iterate each guidance
			float energy = 0.0f;
			float minEnergy = 0.0f;
			cv::Vec3f sourceRGBAvg(0.0);
			cv::Point2f minP;
			bool isFind = false;

			// Iterate each pixel p in A
			for (size_t x_p = 0; x_p < heightOfSource; ++x_p) {
				if (isFind) break;
				for (size_t y_p = 0; y_p < widthOfTarget; ++y_p) {
					// Searching neighbors
					float energyP = 0.0f;
					for (size_t x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
						for (size_t y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
							float gaussian_weight = gaussian[x_neigh + 2][y_neigh + 2];
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

							energyP += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
								rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
								sourceStyle, targetStyle,
								x_p_neigh, y_p_neigh, x_q_neigh, y_q_neigh);
						}
					}

					if (energyP <= k && visited.find(x_p + "," + y_p) == visited.end()) {
						minEnergy = energyP;
						minP = cv::Point2f(x_p, y_p);
						visited.insert(x_p + "," + y_p);
						isFind = true;
						break;
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
			targetStyle_new.at<cv::Vec3f>(x_q, y_q) = sourceRGBAvg;
			minErr += minEnergy;
		}
	}
	*targetStyle = targetStyle_new.clone();
	return minErr;
}

void Stylit::randomAssignPatch(std::vector<int>& patchVector, int aRow, int aCol, int bRow, int bCol)
{
	patchVector = std::vector<int>(bRow * bCol, 0);
	int scales = max(1, bCol * bRow / aCol / aRow);

	for (int i = 0; i < bCol * bRow; ++i) {
		patchVector[i] = i / scales;
	}

	std::random_shuffle(patchVector.begin(), patchVector.end());

}

int Stylit::propagate(const cv::Mat* rgbSource, const cv::Mat* ld12eSource, const cv::Mat* lddeSource, const cv::Mat* ldeSource, const cv::Mat* lseSource, 
	const cv::Mat* rgbTarget, const cv::Mat* ld12eTarget, const cv::Mat* lddeTarget, const cv::Mat* ldeTarget, const cv::Mat* lseTarget, 
	const cv::Mat* sourceStyle, const cv::Mat* targetStyle, 
	int cur, int horizon, int vertical, int x_q, int y_q,
	int aCols, float &energy)
{
	int aRows = sourceStyle->rows;
	// cur
	int x_p = cur / aCols;
	int y_p = cur % aCols;

	float energy1 = 0;
	for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
		int x_p_neigh = x_p + x_neigh;
		x_p_neigh = x_p_neigh < 0 ? 0 : x_p_neigh;
		x_p_neigh = x_p_neigh >= aRows ? aRows - 1 : x_p_neigh;

		for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
			int y_p_neigh = y_p + y_neigh;
			y_p_neigh = y_p_neigh < 0 ? 0 : y_p_neigh;
			y_p_neigh = y_p_neigh >= aCols ? aCols - 1 : y_p_neigh;
			float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

			energy1 += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
				rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
				sourceStyle, targetStyle,
				x_p_neigh, y_p_neigh, x_q, y_q);
		}
	}
	// horizon
	x_p = horizon / aCols;
	y_p = horizon % aCols;

	float energy2 = 0;
	for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
		int x_p_neigh = x_p + x_neigh;
		x_p_neigh = x_p_neigh < 0 ? 0 : x_p_neigh;
		x_p_neigh = x_p_neigh >= aRows ? aRows - 1 : x_p_neigh;

		for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
			int y_p_neigh = y_p + y_neigh;
			y_p_neigh = y_p_neigh < 0 ? 0 : y_p_neigh;
			y_p_neigh = y_p_neigh >= aCols ? aCols - 1 : y_p_neigh;
			float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

			energy2 += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
				rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
				sourceStyle, targetStyle,
				x_p_neigh, y_p_neigh, x_q, y_q);
		}
	}
	// vertical
	x_p = vertical / aCols;
	y_p = vertical % aCols;

	float energy3 = 0;
	for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
		int x_p_neigh = x_p + x_neigh;
		x_p_neigh = x_p_neigh < 0 ? 0 : x_p_neigh;
		x_p_neigh = x_p_neigh >= aRows ? aRows - 1 : x_p_neigh;

		for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
			int y_p_neigh = y_p + y_neigh;
			y_p_neigh = y_p_neigh < 0 ? 0 : y_p_neigh;
			y_p_neigh = y_p_neigh >= aCols ? aCols - 1 : y_p_neigh;
			float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

			energy3 += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
				rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
				sourceStyle, targetStyle,
				x_p_neigh, y_p_neigh, x_q, y_q);
		}
	}

	// Find the minimum energy
	if (energy1 <= energy2 && energy1 <= energy3) {
		energy = energy1;
		return cur;
	}
	if (energy2 <= energy1 && energy2 <= energy3) {
		energy = energy2;
		return horizon;
	}

	if (energy3 <= energy2 && energy3 <= energy1) {
		energy = energy3;
		return vertical;
	}
	return -1;
}

int Stylit::randomSearch(const cv::Mat* rgbSource, const cv::Mat* ld12eSource, const cv::Mat* lddeSource, const cv::Mat* ldeSource, const cv::Mat* lseSource, 
	const cv::Mat* rgbTarget, const cv::Mat* ld12eTarget, const cv::Mat* lddeTarget, const cv::Mat* ldeTarget, const cv::Mat* lseTarget, 
	const cv::Mat* sourceStyle, const cv::Mat* targetStyle, 
	int x_q, int y_q, float& energy)
{


	int aCols = sourceStyle->cols;
	int aRows = sourceStyle->rows;
	int bCols = targetStyle->cols;
	int bRows = targetStyle->rows;

	int source_index = this->patchMatch[x_q * bCols + y_q];
	int x_p = source_index / aCols;
	int y_p = source_index % aCols;

	int window_size_x = aRows / 2;
	int window_size_y = aCols / 2;

	cv::Vec2i new_patch_coord;
	float energy_min = FLT_MAX;

	while (window_size_x > 1 && window_size_y > 1) {
		for (int count = 0; count < 1; ++count) {
			float x_rand = (rand() % 100 - 50) * 2 / 100.f; // [-1, 1]
			float y_rand = (rand() % 100 - 50) * 2 / 100.f; // [-1, 1]
			//printf("x_rand: %f y_rand: %f\n", x_rand, y_rand);

			int x_new = x_p + x_rand * window_size_x;
			x_new = x_new < 0 ? 0 : x_new;
			x_new = x_new >= aRows ? aRows - 1 : x_new;
			int y_new = y_p + y_rand * window_size_y;
			y_new = y_new < 0 ? 0 : y_new;
			y_new = y_new >= aCols ? aCols - 1 : y_new;

			float energy_tmp = 0;
			for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
				int x_p_neigh = x_new + x_neigh;
				x_p_neigh = x_p_neigh < 0 ? 0 : x_p_neigh;
				x_p_neigh = x_p_neigh >= aRows ? aRows - 1 : x_p_neigh;


				for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {

					int y_p_neigh = y_new + y_neigh;
					y_p_neigh = y_p_neigh < 0 ? 0 : y_p_neigh;
					y_p_neigh = y_p_neigh >= aCols ? aCols - 1 : y_p_neigh;
					float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

					energy_tmp += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
						rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
						sourceStyle, targetStyle,
						x_p_neigh, y_p_neigh, x_q, y_q);
				}
			}
			if (energy_tmp < energy_min) {
				energy_min = energy_tmp;
				new_patch_coord = cv::Vec2i(x_new, y_new);
			}
		}

		window_size_x /= 2;
		window_size_y /= 2;
	}

	float energy_cur = 0;
	for (int x_neigh = -bound; x_neigh <= bound; ++x_neigh) {
		int x_p_neigh = x_p + x_neigh;
		x_p_neigh = x_p_neigh < 0 ? 0 : x_p_neigh;
		x_p_neigh = x_p_neigh >= aRows ? aRows - 1 : x_p_neigh;

		for (int y_neigh = -bound; y_neigh <= bound; ++y_neigh) {
			int y_p_neigh = y_p + y_neigh;
			y_p_neigh = y_p_neigh < 0 ? 0 : y_p_neigh;
			y_p_neigh = y_p_neigh >= aCols ? aCols - 1 : y_p_neigh;
			float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

			energy_cur += gaussian_weight * NNF(rgbSource, ld12eSource, lddeSource, ldeSource, lseSource,
				rgbTarget, ld12eTarget, lddeTarget, ldeTarget, lseTarget,
				sourceStyle, targetStyle,
				x_p_neigh, y_p_neigh, x_q, y_q);
		}
	}

	if (energy_cur > energy_min) {
		energy = energy_min;
		return new_patch_coord[0] * aCols + new_patch_coord[1];
	}
	else {
		energy = energy_cur;
		return source_index;
	}

	return -1;
}



void Stylit::setA(std::unique_ptr<Pyramid> a)
{
	this->a = std::move(a);
}

void Stylit::setAP(std::unique_ptr<Pyramid> ap)
{
	this->ap = std::move(ap);
}

void Stylit::setB(std::unique_ptr<Pyramid> b)
{
	this->b = std::move(b);
}

void Stylit::setNeigbor(int neigh)
{
	this->neighborSize = neigh;
	bound = neigh / 2;
}

void Stylit::setMIU(float miu)
{
	this->miu = miu;
}

// Return the energy of a pixel q in image B'
float Stylit::NNF(const cv::Mat* rgbSource, const cv::Mat* ld12eSource, const cv::Mat* lddeSource, const cv::Mat* ldeSource, const cv::Mat* lseSource,
				  const cv::Mat* rgbTarget, const cv::Mat* ld12eTarget, const cv::Mat* lddeTarget, const cv::Mat* ldeTarget, const cv::Mat* lseTarget,
				  const cv::Mat* sourceStyle, const cv::Mat* targetStyle,
				  int x_s, int y_s, int x_t, int y_t)
{
	float rgbEnergy = length(rgbSource->at<cv::Vec3f>(x_s, y_s), rgbTarget->at<cv::Vec3f>(x_t, y_t));
	float guidanceEnergy =  rgbEnergy+
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
			float gaussian_weight = gaussian[x_neigh + bound][y_neigh + bound];

			//printf("x: %d, y: %d\n", x_neigh, y_neigh);

			int x_p_neigh = x_p + x_neigh;
			int y_p_neigh = y_p + y_neigh;

			if (x_p_neigh < 0) x_p_neigh = 0;
			if (x_p_neigh >= heightOfSource) x_p_neigh = heightOfSource - 1;
			if (y_p_neigh < 0) y_p_neigh = 0;
			if (y_p_neigh >= widthOfSource) y_p_neigh = widthOfSource - 1;
			//printf("x1: %d, y1: %d\n", x_p_neigh, y_p_neigh);

			sourceRGBAvg += gaussian_weight * sourceStyle->at<cv::Vec3f>(x_p_neigh, y_p_neigh);

		}
	}
	//sourceRGBAvg = sourceStyle->at<cv::Vec3f>(x_p, y_p);
	//sourceRGBAvg /= float(neighborSize * neighborSize);
	//printf("sourceRGBAvg: %f %f %f\n\n", sourceRGBAvg[0], sourceRGBAvg[1], sourceRGBAvg[2]);

}
