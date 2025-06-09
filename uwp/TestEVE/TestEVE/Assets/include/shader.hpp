#ifndef __SHADER_HPP__
#define __SHADER_HPP__ 1

#ifdef RENDER_OPENGL
#include "GL/glew.h"
#endif
#include "defines.hpp"
#include <iostream>

/**
 * @brief Structure that wraps the functionality of an opengl shader
 */
struct Shader {

  /** Opengl id of the generated shader */
  unsigned int id_;
  /** Type of shader */
  ShaderType type_;

  Shader(DefaultShaderSource source);
  Shader(const char* shader_source, ShaderType type);
  ~Shader();

  /**
   * @brief Reads a plain text file and returns its content as a string
   * 
   * @param filename Path to the file to read
   * 
   * @return char* A pointer to a string with the file content 
   */
  static char* ReadShaderFromFile(const char* filename);

};


#endif // __SHADER_HPP__