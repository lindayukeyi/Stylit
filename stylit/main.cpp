
#include "stylit.h"

#include <iostream>
using namespace std;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<string> lpe = { "beauty", "LDE", "LSE", "LDDE", "LD12E", "style2"};

int main()
{

    // TEST
    std::vector<unique_ptr<cv::Mat>> images(11);
    cv::Mat img = cv::imread("images/sourceppt/style2.jpg");
    cv::Size stylesize = img.size() * 4;
    for (int i = 0; i < 6; i++)
    {
        cv::Mat img = cv::imread("images/sourceppt/" + lpe[i] + ".jpg");
        cv::resize(img, img, stylesize);
        cv::imwrite("images/source_" + lpe[i] + ".jpg", img);

        cv::Mat imgNormalized;
        img.convertTo(imgNormalized, CV_32FC3);
        imgNormalized /= 255.0f;
        images[i] = make_unique<cv::Mat>(imgNormalized);
    }

    for (int i = 0; i < 5; i++)
    {
        cv::Mat img = cv::imread("images/target_torus/target_" + lpe[i] + ".png");
        cv::resize(img, img, stylesize);
        cv::imwrite("images/target_" + lpe[i] + ".jpg", img);

        cv::Mat imgNormalized;
        img.convertTo(imgNormalized, CV_32FC3);
        imgNormalized /= 255.0f;
        images[i + 6] = make_unique<cv::Mat>(imgNormalized);
    }

    unique_ptr<FeatureVector> fa = make_unique<FeatureVector>(images[0], images[1], images[2], images[3], images[4]);
    unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
    unique_ptr<FeatureVector> fap = make_unique<FeatureVector>(images[5], lde, lse, ldde, ld12e);
    unique_ptr<FeatureVector> fb = make_unique<FeatureVector>(images[6], images[7], images[8], images[9], images[10]);
    unique_ptr<Pyramid> pa = make_unique<Pyramid>(fa, 4);
    unique_ptr<Pyramid> pap = make_unique<Pyramid>(fap, 4);
    unique_ptr<Pyramid> pb = make_unique<Pyramid>(fb, 4);

    Stylit stylit_image(std::move(pa), std::move(pap), std::move(pb), 5, 2);
    string tmpp = "./images/";
    stylit_image.setTmpPath(tmpp);

    stylit_image.synthesize();

}