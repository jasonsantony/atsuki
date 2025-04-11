#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include "shader_utils.h"
#include "render_pass.h"

#include <iostream>

// Fullscreen quad vertices (position + texcoord)
float quadVertices[] = {
    // positions  // texcoords
    -1.0f, 1.0f,  0.0f, 1.0f,

    -1.0f, -1.0f, 0.0f, 0.0f,

    1.0f,  1.0f,  1.0f, 1.0f,

    1.0f,  -1.0f, 1.0f, 0.0f,
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

  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <input_image_path> <shader_dir>\n";
    return 1;
  }

  std::string inputPath = argv[1];
  std::string shaderDir = argv[2];

  // Load image using OpenCV
  cv::Mat inputImage = cv::imread(inputPath);
  if (inputImage.empty()) {
    std::cerr << "Failed to load input image" << std::endl;
    return -1;
  }
  int renderWidth = inputImage.cols;
  int renderHeight = inputImage.rows;

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(renderWidth, renderHeight, "ATSUKI", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetWindowAspectRatio(window, renderWidth, renderHeight);

  if (!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  GLuint quadVAO = createFullscreenQuadVAO();
  GLuint inputTex = loadTextureFromImage(inputImage);

  // Setup render passes
  RenderPass edgePass, histPass, asciiPass;
  edgePass.init(renderWidth, renderHeight, shaderDir + "/fullscreen_quad.vert",
                shaderDir + "/edge_detect.frag");
  histPass.init(renderWidth, renderHeight, shaderDir + "/fullscreen_quad.vert",
                shaderDir + "/histogram.frag");
  asciiPass.init(renderWidth, renderHeight, shaderDir + "/fullscreen_quad.vert",
                 shaderDir + "/ascii_render.frag");

  // Display pass (simple passthrough shader)
  ShaderProgram displayShader(shaderDir + "/fullscreen_quad.vert",
                              shaderDir + "/display.frag");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    edgePass.run(inputTex, quadVAO, [&](GLuint program) {
      glUniform1f(glGetUniformLocation(program, "u_threshold"), 0.2f);
      glUniform2f(glGetUniformLocation(program, "u_textureSize"),
                  (float)renderWidth, (float)renderHeight);
    });

    histPass.run(edgePass.texture, quadVAO, [&](GLuint program) {
      glUniform2f(glGetUniformLocation(program, "u_gridCellDimensions"), 40.0f,
                  25.0f); // example: 40 cols, 25 rows
      glUniform1i(glGetUniformLocation(program, "u_threshold"), 10);
    });

    // Final display
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    displayShader.reloadIfChanged();
    glUseProgram(displayShader.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, histPass.texture);
    glUniform1i(glGetUniformLocation(displayShader.id, "image"), 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}
