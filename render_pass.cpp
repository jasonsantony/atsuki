#include "render_pass.h"
#include <glad/gl.h>
#include <iostream>
#include <memory>

GLuint createRenderTexture(int width, int height) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  // Allocate an empty high-precision floating-point RGB texture
  // This will serve as the render target for shader outputs in multi-pass
  // rendering
  glTexImage2D(
      GL_TEXTURE_2D, // Target: 2D texture
      0,             // Mipmap level: 0 (base level)
      GL_RGB16F,     // Internal format: 16-bit floating point RGB per channel
      width, height, // Texture dimensions
      0,             // Border: must be 0
      GL_RGB,        // Format of pixel data: RGB
      GL_FLOAT,      // Type of pixel data: float per component
      nullptr        // No initial data (allocate only)
  );
  // Linear interpolation for downsampling (we don't need upsampling)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return tex;
}

GLuint createFramebuffer(GLuint tex) {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  // Attach the texture to the framebuffer's color attachment point (slot 0)
  // This allows rendering output to be stored in the texture
  glFramebufferTexture2D(
      GL_FRAMEBUFFER,       // Target framebuffer
      GL_COLOR_ATTACHMENT0, // Attachment point (color buffer 0)
      GL_TEXTURE_2D,        // Texture target type
      tex,                  // Texture to attach
      0                     // Mipmap level (0 = base)
  );
  return fbo;
}

void RenderPass::init(int w, int h, const std::string &fragPath) {
  this->width = w;
  this->height = h;
  this->texture = createRenderTexture(w, h);
  this->fbo = createFramebuffer(this->texture);
  this->shader =
      std::make_unique<ShaderProgram>("shaders/fullscreen_quad.vert", fragPath);
  return;
}

void RenderPass::run(GLuint inputTex, GLuint vao) {
  this->shader->reloadIfChanged();
  glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
  glViewport(0, 0, this->width, this->height);
  // clear the color (2D) buffer of the currently bound framebuffer
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(this->shader->id);

  // Bind input texture to texture unit 0
  glActiveTexture(GL_TEXTURE0);           // Select texture unit 0
  glBindTexture(GL_TEXTURE_2D, inputTex); // Bind inputTex to the selected unit
  glUniform1i(glGetUniformLocation(this->shader->id, "image"),
              0); // Tell shader that 'image' refers to texture unit 0

  // Bind the VAO which holds the vertex attributes for a fullscreen quad
  glBindVertexArray(vao);
  // Draw 4 vertices as a triangle strip (2 triangles forming a fullscreen quad)
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

RenderPass::~RenderPass() {
  glDeleteTextures(1, &texture);
  glDeleteFramebuffers(1, &fbo);
  // shader cleanup is handled by unique_ptr
}