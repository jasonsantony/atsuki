#pragma once
#include <string>
#include <filesystem>
#include <glad/gl.h>

struct ShaderProgram {
  GLuint id;
  std::string vertPath, fragPath;
  // Allows automatic shader recompililation if you edit & save the .frag file
  // while app is running
  std::filesystem::file_time_type lastFragTime;

  ShaderProgram(const std::string &vp, const std::string &fp);
  ~ShaderProgram();
  void reloadIfChanged();
};