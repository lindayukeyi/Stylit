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

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("simple.vert.glsl", "simple.frag.glsl");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load("images/wood.png", &width, &height, &nrChannels, 0);
    cv::Mat M(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
    image input("images/wood.png");
    input.savePNG("images/op.png");
    cv::Mat img = cv::imread("images/wood.png");
    cv::imshow("wood", img);
    cv::Mat imgNormalized;
    img.convertTo(imgNormalized, CV_32FC1);
    imgNormalized /= 255.0f;
    Stylit stylit(5);
    cv::Vec3f avg(0.0f);
    stylit.averageColor(&imgNormalized, &M, 0, 0, 32, 32, avg);

    // TEST
    cv::Mat img1 = cv::imread("images/wood.png");
    cv::Mat img2 = cv::imread("images/wood.png");
    cv::Mat img3 = cv::imread("images/wood.png");
    cv::Mat img4 = cv::imread("images/wood.png");
    cv::Mat img5 = cv::imread("images/wood.png");
    cv::Mat img6 = cv::imread("images/wood.png");
    cv::Mat img7 = cv::imread("images/wood.png");
    cv::Mat img8 = cv::imread("images/wood.png");
    cv::Mat img9 = cv::imread("images/wood.png");
    cv::Mat img10 = cv::imread("images/wood.png");
    cv::Mat img11 = cv::imread("images/wood.png");
    unique_ptr<cv::Mat> image1 = make_unique<cv::Mat>(img1);
    unique_ptr<cv::Mat> image2 = make_unique<cv::Mat>(img2);
    unique_ptr<cv::Mat> image3 = make_unique<cv::Mat>(img3);
    unique_ptr<cv::Mat> image4 = make_unique<cv::Mat>(img4);
    unique_ptr<cv::Mat> image5 = make_unique<cv::Mat>(img5);
    unique_ptr<cv::Mat> image6 = make_unique<cv::Mat>(img6);
    unique_ptr<cv::Mat> image7 = make_unique<cv::Mat>(img7);
    unique_ptr<cv::Mat> image8 = make_unique<cv::Mat>(img8);
    unique_ptr<cv::Mat> image9 = make_unique<cv::Mat>(img9);
    unique_ptr<cv::Mat> image10 = make_unique<cv::Mat>(img10);
    unique_ptr<cv::Mat> image11 = make_unique<cv::Mat>(img11);
    unique_ptr<FeatureVector> fa = make_unique<FeatureVector>(image1, image2, image3, image4, image5);
    unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
    unique_ptr<FeatureVector> fap = make_unique<FeatureVector>(image6, lde, lse, ldde, ld12e);
    unique_ptr<FeatureVector> fb = make_unique<FeatureVector>(image7, image8, image9, image10, image11);
    unique_ptr<Pyramid> pa = make_unique<Pyramid>(fap, 2);
    unique_ptr<Pyramid> pap = make_unique<Pyramid>(fa, 2);
    unique_ptr<Pyramid> pb = make_unique<Pyramid>(fb, 2);

    Stylit stylit_image(pa, pap, pb, 5);
   // stylit_image.synthesize();

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // transformation

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 0.0f));

    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);

        //ourShader.setMat4("model", model);
        //ourShader.setMat4("projection", projection);
        //ourShader.setMat4("view", view);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}