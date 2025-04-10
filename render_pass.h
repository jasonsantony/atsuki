#pragma once
#include <string>
#include "shader_utils.h"
#include <memory>

struct RenderPass {
  std::unique_ptr<ShaderProgram> shader;
  GLuint fbo, texture;
  int width, height;

  // We use init instead of constructor because of OpenGL obj ownership
  // May need to be created before all OpenGL resources are available
  void init(int w, int h, const std::string &vertPath,
            const std::string &fragPath);
  void run(GLuint inputTex, GLuint vao);
  ~RenderPass(); // Automatically clean up GL resources on destruction
};