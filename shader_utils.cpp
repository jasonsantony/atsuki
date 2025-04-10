#include "shader_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const std::string &path) {
  std::ifstream file(path);
  std::cout << "Loading shader from: " << path << std::endl;
  if (!file.is_open()) {
    std::cerr << "Failed to open shader file: " << path << std::endl;
    return ""; // <- avoid passing garbage to compiler
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static GLuint compileShader(GLenum type, const std::string &src) {
  GLuint shader = glCreateShader(type);
  const char *csrc = src.c_str();
  glShaderSource(shader, 1, &csrc, nullptr);
  glCompileShader(shader);

  // Check for successful compilation
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(shader, 512, nullptr, log);
    std::cerr << "Shader compilation error:\n" << log << std::endl;
  }
  return shader;
}

ShaderProgram::ShaderProgram(const std::string &vp, const std::string &fp)
    : vertPath(vp), fragPath(fp),
      lastFragTime(std::filesystem::last_write_time(fp)) {
  std::string vertSrc = readFile(vp);
  std::string fragSrc = readFile(fp);

  GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
  GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

  this->id = glCreateProgram();
  glAttachShader(this->id, vert);
  glAttachShader(this->id, frag);
  glLinkProgram(this->id);
  GLint success;
  glGetProgramiv(this->id, GL_LINK_STATUS, &success);
  if (!success) {
    char log[512];
    glGetProgramInfoLog(this->id, 512, nullptr, log);
    std::cerr << "Shader program link error:\n" << log << std::endl;
  }

  glDeleteShader(vert);
  glDeleteShader(frag);
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(id); }

void ShaderProgram::reloadIfChanged() {
  auto newTime = std::filesystem::last_write_time(this->fragPath);
  if (newTime != lastFragTime) {
    std::cout << "Reloading shader: " << fragPath << std::endl;
    glDeleteProgram(id);

    std::string vertSrc = readFile(this->vertPath);
    std::string fragSrc = readFile(this->fragPath);

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    this->id = glCreateProgram();
    glAttachShader(this->id, vert);
    glAttachShader(this->id, frag);
    glLinkProgram(this->id);
    GLint success;
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if (!success) {
      char log[512];
      glGetProgramInfoLog(this->id, 512, nullptr, log);
      std::cerr << "Shader program reload link error:\n" << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    lastFragTime = newTime;
  }
}
