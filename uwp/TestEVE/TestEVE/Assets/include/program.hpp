#ifndef __PROGRAM_HPP__
#define __PROGRAM_HPP__ 1

#include <iostream>
#include <glm/glm.hpp>

#include "defines.hpp"

#include "shader.hpp"
#include <unordered_map>

/**
 * @brief Structure that wrapps around the functionality of a opengl program
 */
struct Program {

  /** Opengl id of the generated program */
#ifdef RENDER_OPENGL
  GLuint id_;
#endif

  /** Reference to the fragement shader */
  Shader* fragment_shader_;
  /** Reference to the vertex shader */
  Shader* vertex_shader_;
  /** Reference to the geometry shader */
  Shader* geometry_shader_;

  /** Unordered map that holds the pre-cached locations of each uniform used in the shader to prevent repeated calls to "glGetUniformLocation" and improve efficiency */

#ifdef RENDER_OPENGL
  std::unordered_map<std::string, GLuint> uniform_locations_;
#endif

  Program();
  ~Program();

  /**
   * @brief Attach a given shader to the program
   * 
   * @param shader Which shader to attach to the program
   */
  void AttachShader(Shader* shader);

  /**
   * @brief Detach given shader of the program and clear the uniform pre-caching
   *
   * @param shader Which shader to detach from the program
   */
  void DetachShader(Shader* shader);

  /**
   * @brief Link the program to the current opengl implementation
   * 
   * @return bool False if the link fails and displays via console an error message with the compilation error code
   *         True If the linking succeds
   */
  bool Link();

  /**
   * @brief Sets this program as the used in the rendering state of the 
   * 
   */
  void Use();

  /**
   * @brief Sets a boolean value on the uniform that will use it's name on the shader
   * 
   * @param name Which uniform to set identified by name
   * @param value Value to set on the uniform
   */
  void SetBool(const char* name, bool value);

  /**
    * @brief Sets an int value on the uniform that will use it's name on the shader
    *
    * @param name Which uniform to set identified by name
    * @param value Value to set on the uniform
    */
  void SetInt(const char* name, int value);

  /**
    * @brief Sets an unsigend int value on the uniform that will use it's name on the shader
    *
    * @param name Which uniform to set identified by name
    * @param value Value to set on the uniform
    */
  void SetUInt(const char* name, unsigned int value);

  /**
    * @brief Sets a float value on the uniform that will use it's name on the shader
    *
    * @param name Which uniform to set identified by name
    * @param value Value to set on the uniform
    */
  void SetFloat(const char* name, float value);

  /**
    * @brief Sets a vec2 value on the uniform that will use it's name on the shader
    *
    * @param name Which uniform to set identified by name
    * @param value Pointer to the values to set on the uniform
    */
  void SetVec2(const char* name, float* value);

  /**
    * @brief Sets a vec2 value on the uniform that will use it's name on the shader
    *
    * @param name Which uniform to set identified by name
    * @param v1 Value of the first float of the vec2 to set
    * @param v2 Value of the second float of the vec2 to set
    */
  void SetVec2(const char* name, float v1, float v2);

  /**
   * @brief Sets a vec2 value on the uniform that will use it's name on the shader
   * 
   * @param name Which uniform to set identified by name
   * @param value Value to set on the uniform
   */
  void SetVec2(const char* name, glm::vec2 value);

  /**
   *  @brief Sets a vec3 value on the uniform that will use it's name on the shader
   * 
   * @param name Which uniform to set identified by name
   * @param value Pointer to the values to set on the uniform
   */
  void SetVec3(const char* name, float* value);

  /**
  * @brief Sets a vec3 value on the uniform that will use it's name on the shader
  *
  * @param name Which uniform to set identified by name
  * @param v1 Value of the first float of the vec2 to set
  * @param v2 Value of the second float of the vec2 to set
  * @param v3 Value of the third float of the vec2 to set
  */
  void SetVec3(const char* name, float v1, float v2, float v3);

  /**
   * @brief Sets a vec3 value on the uniform that will use it's name on the shader
   *
   * @param name Which uniform to set identified by name
   * @param value Value to set on the uniform
   */
  void SetVec3(const char* name, glm::vec3 value);

  /**
   * @brief Sets a mat4 value on the uniform that will use it's name on the shader
   * 
   * @param name Which uniform to set identified by name
   * @param value Pointer to the values to set on the uniform
   */
  void SetMat4(const char* name, float* value);

  /**
   * @brief Sets a texture unity into the uniform that will use it's name on the shader
   * 
   * @param name Which uniform to set identified by name
   * @param texture_unit Which texture unit will be associated with the texture
   */
  void SetSampler(const char* name, int texture_unit);

  /**
   * @brief Retrieves the uniform location associated with a name previously pre-cached into a map. 
   * The first time it's used with a given name, it searches for it and saves it, 
   * and the subsequent times it only retrieves that value without calling glGetUniformLocation
   * 
   * Because of that, it's recommended to <B>avoid changing the attached shaders 
   * after it's initialization without clearing the unordered_map</B>
   * 
   * 
   * @param name What uniform location to retrieve
   * @return GLuint which location id associated with the uniform
   *         -1 if name does not correspond to an active uniform variable in program, 
   *            if name starts with the reserved prefix "gl_", or if name is associated 
   *            with an atomic counter or a named uniform block
   */

#ifdef RENDER_OPENGL
  GLuint GetUniformLocation(const char* name);
#endif

};

#endif // __PROGRAM_HPP__