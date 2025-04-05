#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "utils/shader_loader.hpp"

int main() {

  std::cout << R"(
 ______   ______  ______   __  __   __  __   __    
/\  __ \ /\__  _\/\  ___\ /\ \/\ \ /\ \/ /  /\ \   
\ \  __ \\/_/\ \/\ \___  \\ \ \_\ \\ \  _"-.\ \ \  
 \ \_\ \_\  \ \_\ \/\_____\\ \_____\\ \_\ \_\\ \_\ 
  \/_/\/_/   \/_/  \/_____/ \/_____/ \/_/\/_/ \/_/ 
      ASCII  EFFECT  FILTER  FOR  YOUR  VIDEOS     
  )" << std::endl;

  if (!glfwInit) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(640, 480, "ATSUKI", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to load GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }

  cv::VideoCapture cap("assets/video.mp4");
  if (!cap.isOpened()) {
    std::cerr << "Failed to open assets/video.mp4" << std::endl;
    glfwTerminate();
    return -1;
  }
  int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

  cv::VideoWriter writer("output/video.mp4",
                         cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
                         cap.get(cv::CAP_PROP_FPS), cv::Size(width, height));
  if (!writer.isOpened()) {
    std::cerr << "Failed to open output/video.mp4 for writing" << std::endl;
    cap.release();
    glfwTerminate();
    return -1;
  }

  // TODO: implement utils
  // TODO: implement shaders
  GLuint shader = LoadShader("shaders/shader.glsl");

  GLuint texture;
  glGenTextures(1, &texture);            // create texture on GPU
  glBindTexture(GL_TEXTURE_2D, texture); // currently active 2d texture
  // linear interpolation scaling
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // x and y in [-1.0, 1.0] (where to draw on screen)
  // u and v in [0.0, 1.0] (texture space)
  // this is trivial example, but generally we project (u, v) onto (x, y)
  float vertices[] = {
      //  x     y      u     v
      -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
      1.0f,  -1.0f, 1.0f, 0.0f, // Bottom-right
      1.0f,  1.0f,  1.0f, 1.0f, // Top-right
      -1.0f, 1.0f,  0.0f, 1.0f  // Top-left
  };
  unsigned int indices[] = {
      0, 1, 2, // bottom-left -> bottom-right -> top-right
      2, 3, 0  // top-right -> top-left -> bottom-left
  }; // define 2 triangles using vertices -- wound counter-clockwise

  // vertex (config) array, vertex buffer, element (index) buffer
  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // send config to GPU
  glBindVertexArray(VAO);

  // send vertices to GPU
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // send indices to GPU
  glBindBuffer(GL_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // tell gpu how to interpret flattened vertex buffer
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  // attribute location 0 -> position (x, y)
  // stride of 16 bytes (4 floats in each vertex)
  // reads 2 floats, starts at byte offset 0 in each vertex
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  // attribute location 1 -> texture coordinates (u, v)
  // stride of 16 bytes
  // also 2 floats, starts at byte offset 8 (after x, y)
  glEnableVertexAttribArray(1);

  cv::Mat frame;
  cv::Mat outputFrame(height, width, CV_8UC3); // 3 GL_UNSIGNED_BYTE(S) for RGB
  while (!glfwWindowShouldClose(window)) {
    if (!cap.read(frame))
      break;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    // upload frame as texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, frame.data);

    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
    glUseProgram(shader);
    glBindVertexArray(VAO); // remember config (good practice)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                   (void *)0); // texture -> screen
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                 outputFrame.data);
    cv::cvtColor(outputFrame, outputFrame, cv::COLOR_RGB2BGR);
    cv::flip(outputFrame, outputFrame,
             0); // TODO: we can do this with a shader later
    writer.write(outputFrame);

    glfwSwapBuffers(window); // swap front and back buffer of window to update
    glfwPollEvents();        // up to date loop condition
  }

  // cleanup (OS handles in this case)
  cap.release();
  writer.release();
  glDeleteProgram(shader);
  glDeleteTextures(1, &texture);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glfwTerminate();
  return 0;
}
