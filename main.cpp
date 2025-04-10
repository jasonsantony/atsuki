#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include "shader_utils.h"
#include "render_pass.h"

#include <iostream>

// Fullscreen quad vertices (position + texcoord)
float quadVertices[] = {
    // positions   // texcoords
    -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f,
};

GLuint createFullscreenQuadVAO() {
  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  return VAO;
}

GLuint loadTextureFromImage(const cv::Mat &image) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  cv::Mat flipped;
  cv::flip(image, flipped, 0); // OpenGL expects origin at bottom-left

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flipped.cols, flipped.rows, 0, GL_BGR,
               GL_UNSIGNED_BYTE, flipped.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return tex;
}

int main(int argc, char **argv) {
  std::cout << R"(
    ______   ______  ______   __  __   __  __   __    
   /\  __ \ /\__  _\/\  ___\ /\ \/\ \ /\ \/ /  /\ \   
   \ \  __ \\/_/\ \/\ \___  \\ \ \_\ \\ \  _"-.\ \ \  
    \ \_\ \_\  \ \_\ \/\_____\\ \_____\\ \_\ \_\\ \_\ 
     \/_/\/_/   \/_/  \/_____/ \/_____/ \/_/\/_/ \/_/ 
         ASCII  EFFECT  FILTER  FOR  YOUR  VIDEOS
  )" << std::endl;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input_image_path>\n";
    return 1;
  }

  std::string inputPath = std::string(SOURCE_DIR) + "/" + argv[1];

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int initialWidth = 1280, initialHeight = 720;
  GLFWwindow *window =
      glfwCreateWindow(initialWidth, initialHeight, "ATSUKI", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  GLuint quadVAO = createFullscreenQuadVAO();

  // Load image using OpenCV
  cv::Mat inputImage = cv::imread(inputPath);
  if (inputImage.empty()) {
    std::cerr << "Failed to load input image" << std::endl;
    return -1;
  }
  GLuint inputTex = loadTextureFromImage(inputImage);

  // Display pass (simple passthrough shader)
  ShaderProgram displayShader(
      std::string(SOURCE_DIR) + "/shaders/fullscreen_quad.vert",
      std::string(SOURCE_DIR) + "/shaders/display.frag");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    float imageAspect = static_cast<float>(inputImage.cols) / inputImage.rows;
    float windowAspect = static_cast<float>(windowWidth) / windowHeight;

    int viewWidth = windowWidth;
    int viewHeight = windowHeight;
    if (windowAspect > imageAspect) {
      viewWidth = static_cast<int>(windowHeight * imageAspect);
      viewHeight = windowHeight;
    } else {
      viewWidth = windowWidth;
      viewHeight = static_cast<int>(windowWidth / imageAspect);
    }

    int xOffset = (windowWidth - viewWidth) / 2;
    int yOffset = (windowHeight - viewHeight) / 2;

    // Draw image to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(xOffset, yOffset, viewWidth, viewHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    displayShader.reloadIfChanged();
    glUseProgram(displayShader.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    glUniform1i(glGetUniformLocation(displayShader.id, "image"), 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}
