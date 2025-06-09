#include "program.hpp"

#ifdef RENDER_OPENGL
Program::Program() {
  id_ = glCreateProgram();
  fragment_shader_ = nullptr;
  vertex_shader_ = nullptr;
  geometry_shader_ = nullptr;
}

Program::~Program() {
  glDeleteProgram(id_);
}

void Program::AttachShader(Shader* shader) {
  glAttachShader(id_, shader->id_);
  switch (shader->type_) {
    case ShaderType::FRAGMENT:
      fragment_shader_ = shader;
    break;
    case ShaderType::VERTEX:
      vertex_shader_ = shader;
    break;
    case ShaderType::GEOMETRY:
      geometry_shader_= shader;
    break;
  }
}

void Program::DetachShader(Shader* shader){
  glDetachShader(id_, shader->id_);
  uniform_locations_.clear();
}

bool Program::Link() {
  glLinkProgram(id_);
  int success = -1;
  char log[512] = "\0";
  glGetProgramiv(id_, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id_, 512, NULL, log);
    std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << log << std::endl;
    return false;
  }
  return true;
}

void Program::Use() {
  glUseProgram(id_);
}

void Program::SetBool(const char* name, bool value) {
  GLint loc = GetUniformLocation(name);
  glUniform1i(loc, (int)value);
}

void Program::SetInt(const char* name, int value) {
  GLint loc = GetUniformLocation(name);
  glUniform1i(loc, value);
}

void Program::SetUInt(const char* name, unsigned int value) {
  GLint loc = GetUniformLocation(name);
  glUniform1ui(loc, value);
}

void Program::SetFloat(const char* name, float value) {
  GLint loc = GetUniformLocation(name);
  glUniform1f(loc, value);
}

void Program::SetVec2(const char* name, float* value) {
  GLint loc = GetUniformLocation(name);
  glUniform2f(loc, value[0], value[1]);
}

void Program::SetVec2(const char* name, float value1, float value2) {
  GLint loc = GetUniformLocation(name);
  glUniform2f(loc, value1, value2);
}

void Program::SetVec2(const char* name, glm::vec2 value) {
  GLint loc = GetUniformLocation(name);
  glUniform2f(loc, value.x, value.y);
}

void Program::SetVec3(const char* name, float* value) {
  GLint loc = GetUniformLocation(name);
  glUniform3f(loc, value[0], value[1], value[2]);
}

void Program::SetVec3(const char* name, float value1, float value2, float value3) {
  GLint loc = GetUniformLocation(name);
  glUniform3f(loc, value1, value2, value3);
}

void Program::SetVec3(const char* name, glm::vec3 value) {
  GLint loc = GetUniformLocation(name);
  glUniform3f(loc, value.x, value.y, value.z);
}

void Program::SetMat4(const char* name, float* value) {
  GLint loc = GetUniformLocation(name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, value);
}

void Program::SetSampler(const char* name, int texture_unit){
  GLint loc = GetUniformLocation(name);
  glUniform1i(loc, texture_unit);
}

//Used to precache the uniform locations used in the program, and speed up 
//subsequent calls asking for a previously loaded location
GLuint Program::GetUniformLocation(const char* name){

  //If not found, register it
  GLuint location_ = 0;
  std::unordered_map<std::string, GLuint>::iterator pos = uniform_locations_.find(name);
  if (pos == uniform_locations_.end()) {
    uniform_locations_[name] = glGetUniformLocation(id_, name);
  }
  location_ = uniform_locations_[name];

  return location_;
}
#endif