#include <iostream>
#include <string>
#include "stb_image_write.h"
#include "std_image.h"

#include "image.h"

image::image(const std::string& filename)
{
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    xSize = width;
    ySize = height;
    pixels = new glm::vec3[width * height];
    int count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            setPixel(x, y, glm::vec3(data[count * 3 + 0] / 255.0, data[count * 3 + 1] / 255.0, data[count * 3 + 2] / 255.0));
            count++;
        }
    }
}

image::image(int x, int y) :
    xSize(x),
    ySize(y),
    pixels(new glm::vec3[x * y]) {
}

image::~image() {
    delete pixels;
}


void image::setPixel(int x, int y, const glm::vec3& pixel) {
    assert(x >= 0 && y >= 0 && x < xSize&& y < ySize);
    pixels[(y * xSize) + x] = pixel;
}

void image::savePNG(const std::string& baseFilename) {
    unsigned char* bytes = new unsigned char[3 * xSize * ySize];
    for (int y = 0; y < ySize; y++) {
        for (int x = 0; x < xSize; x++) {
            int i = y * xSize + x;
            glm::vec3 pix = glm::clamp(pixels[i], glm::vec3(), glm::vec3(1)) * 255.f;
            bytes[3 * i + 0] = (unsigned char)pix.x;
            bytes[3 * i + 1] = (unsigned char)pix.y;
            bytes[3 * i + 2] = (unsigned char)pix.z;
        }
    }

    std::string filename = baseFilename + ".png";
    stbi_write_png(filename.c_str(), xSize, ySize, 3, bytes, xSize * 3);
    std::cout << "Saved " << filename << "." << std::endl;

    delete[] bytes;
}

void image::saveHDR(const std::string& baseFilename) {
    std::string filename = baseFilename + ".hdr";
    stbi_write_hdr(filename.c_str(), xSize, ySize, 3, (const float*)pixels);
    std::cout << "Saved " + filename + "." << std::endl;
}

cv::Mat image::toMat() {
    cv::Mat m(xSize, ySize, CV_8UC1);
    memcpy(m.data, pixels, xSize * ySize * sizeof(uchar));
    return m;
}