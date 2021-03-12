#pragma once

#include <glm/glm/glm.hpp>
#include <string>

using namespace std;

class image {
private:
    int xSize;
    int ySize;
    glm::vec3* pixels;

public:
    image(const std::string &filename);
    image(int x, int y);
    ~image();
    void setPixel(int x, int y, const glm::vec3& pixel);
    void savePNG(const std::string& baseFilename);
    void saveHDR(const std::string& baseFilename);
};
