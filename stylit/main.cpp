#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "std_image.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "image.h"
#include "stylit.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<string> lpe = { "beauty", "LDE", "LSE", "LDDE", "interreflection", "style"};

int main()
{

    // TEST
    std::vector<unique_ptr<cv::Mat>> images(11);
    cv::Mat imgtest = cv::imread("images/wood.png");

    for (int i = 0; i < 6; i++)
    {
        cv::Mat img = cv::imread("images/source/source_" + lpe[i] + ".jpg");
        cv::Mat newimg;
        cv::Size s = img.size();
        s /= 2;
        cv::pyrDown(img, newimg, s);
        s /= 2;
        cv::pyrDown(newimg, newimg, s);
        cv::imwrite("images/source_" + lpe[i] + ".jpg", newimg);
        cv::Mat imgNormalized;
        newimg.convertTo(imgNormalized, CV_32FC3);
        imgNormalized /= 255.0f;
        cv::imshow("sss", imgNormalized);
        int k = cv::waitKey(0);
        if (k == 27) {
             //wait for ESC key to exit
             cv::destroyAllWindows();
        }
        images[i] = make_unique<cv::Mat>(imgNormalized);
    }

    for (int i = 0; i < 5; i++)
    {
        cv::Mat img = cv::imread("images/target_bunny/target_" + lpe[i] + ".jpg");
        cv::Mat newimg;
        cv::Size s = img.size();
        s /= 2;
        cv::pyrDown(img, newimg, s);
        s /= 2;
        cv::pyrDown(newimg, newimg, s);
        cv::imwrite("images/target_" + lpe[i] + ".jpg", newimg);
        cv::Mat imgNormalized;
        newimg.convertTo(imgNormalized, CV_32FC3);
        imgNormalized /= 255.0f;
        cv::imshow("sss", imgNormalized);
        int k = cv::waitKey(0);
        if (k == 27) {
            //wait for ESC key to exit
            cv::destroyAllWindows();
        }
        images[i] = make_unique<cv::Mat>(imgNormalized);
        images[i + 6] = make_unique<cv::Mat>(imgNormalized);
    }

    unique_ptr<FeatureVector> fa = make_unique<FeatureVector>(images[0], images[1], images[2], images[3], images[4]);
    unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
    unique_ptr<FeatureVector> fap = make_unique<FeatureVector>(images[5], lde, lse, ldde, ld12e);
    unique_ptr<FeatureVector> fb = make_unique<FeatureVector>(images[6], images[7], images[8], images[9], images[10]);
    unique_ptr<Pyramid> pa = make_unique<Pyramid>(fa, 1);
    unique_ptr<Pyramid> pap = make_unique<Pyramid>(fap, 1);
    unique_ptr<Pyramid> pb = make_unique<Pyramid>(fb, 1);

    Stylit stylit_image(std::move(pa), std::move(pap), std::move(pb), 5, 2);

    stylit_image.synthesize();

}