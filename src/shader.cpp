#include "shader.hpp"

#ifdef RENDER_OPENGL
Shader::Shader(DefaultShaderSource source) {
  int success = -1;
  char log[512] = "\0";  
  id_ = -1;
  type_ = ShaderType::FRAGMENT;
  const char* src = nullptr;
  switch (source) {
  case DefaultShaderSource::VERTEX:
    id_ = glCreateShader(GL_VERTEX_SHADER);
    type_ = ShaderType::VERTEX;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader.vert");    
    glShaderSource(id_, 1, &src, &success);
    break;
  case DefaultShaderSource::FRAGMENT_TEXTURE:
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    type_ = ShaderType::FRAGMENT;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader_texture.frag");
    glShaderSource(id_, 1, &src, &success);
    break;
  case DefaultShaderSource::FRAGMENT_NORMAL:
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    type_ = ShaderType::FRAGMENT;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader_normal.frag");
    glShaderSource(id_, 1, &src, &success);
    break;  
  case DefaultShaderSource::FRAGMENT_UV:
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    type_ = ShaderType::FRAGMENT;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader_uv.frag");
    glShaderSource(id_, 1, &src, &success);
    break;
  case DefaultShaderSource::FRAGMENT_POSITION:
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    type_ = ShaderType::FRAGMENT;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader_position.frag");
    glShaderSource(id_, 1, &src, &success);
    break;
  case DefaultShaderSource::FRAGMENT_PLAIN_COLOR:
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    type_ = ShaderType::FRAGMENT;
    src = ReadShaderFromFile("../data/shaders_pc/default_shader_color.frag");
    glShaderSource(id_, 1, &src, &success);
    break;
  }

  if(src) delete src;

  glCompileShader(id_);
  glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(id_, 512, nullptr, log);
    switch (type_) {
    case ShaderType::FRAGMENT:
      std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << log << std::endl;
      break;
    case ShaderType::VERTEX:
      std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << log << std::endl;
      break;
    }
  }
}

Shader::Shader(const char* shader_source, ShaderType type) {
  int success = -1;
  char log[512] = "\0";

  type_ = type;
  id_ = -1;
  switch (type_) {
    case ShaderType::FRAGMENT: id_ = glCreateShader(GL_FRAGMENT_SHADER);break;
    case ShaderType::VERTEX: id_ = glCreateShader(GL_VERTEX_SHADER);break;
    case ShaderType::GEOMETRY: id_ = glCreateShader(GL_GEOMETRY_SHADER);break;
  }

  glShaderSource(id_, 1, &shader_source, NULL);
  glCompileShader(id_);
  glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(id_, 512, nullptr, log);
    switch (type_) {
      case ShaderType::FRAGMENT:
        std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << log << std::endl; 
      break;
      case ShaderType::VERTEX:
        std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << log << std::endl; 
      break;
      case ShaderType::GEOMETRY:
        std::cout << "ERROR::GEOMETRY_SHADER::COMPILATION_FAILED\n" << log << std::endl;
      break;
    }
  }
}

Shader::~Shader() {
  glDeleteShader(id_);
}

char* Shader::ReadShaderFromFile(const char* filename) {
  FILE* f;
  fopen_s(&f, filename, "r");
  if ( f == nullptr) {
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* filecontent = (char*)calloc(size + 1, sizeof(char));
  if (filecontent) {
    fread(filecontent, sizeof(char), size, f);
    filecontent[size] = '\0';
  }
  fclose(f);
  return filecontent;
}
#endif