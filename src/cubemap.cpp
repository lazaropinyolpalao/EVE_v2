#include "cubemap.hpp"

#include <stb_image.h>
#include <tinyobj.hpp>

#ifdef RENDER_OPENGL
Cubemap::Cubemap() {

  width_ = 0;
  height_ = 0;
  channels_ = 0;
  selected_texture_ = 0;
  visible_ = true;

  mesh_ = std::make_unique<TinyObj>();
  mesh_->LoadObj("../data/meshes/cube.obj");
  mesh_->InitBuffer();

  std::unique_ptr<char> src_vertex  { Shader::ReadShaderFromFile(CUBEMAP_VERTEX_SHADER) };
  std::unique_ptr<char> src_fragment{ Shader::ReadShaderFromFile(CUBEMAP_FRAGMENT_SHADER) };

  program_ = std::make_unique<Program>();
  program_->AttachShader(std::make_unique<Shader>(src_vertex.get(), ShaderType::VERTEX).get());
  program_->AttachShader(std::make_unique<Shader>(src_fragment.get(), ShaderType::FRAGMENT).get());
  program_->Link();

}


bool Cubemap::addTexture(std::string filepath) {
  if (filepath.empty()) {
    return false;
  }
  std::unique_ptr<Texture> text = std::make_unique<Texture>();
  text->LoadCubemapTexture(filepath);
  textures_.push_back(std::move(text));
  return true;
}
#endif

#ifdef RENDER_DIRECTX11
Cubemap::Cubemap(ID3D11Device* dev){

  width_ = 0;
  height_ = 0;
  channels_ = 0;
  selected_texture_ = 0;
  visible_ = true;

  mesh_ = std::make_unique<TinyObj>();
  mesh_->LoadObj("../data/meshes/cube.obj");
  //mesh_->InitBuffer(dev);

  //TODO: Add shaders
}
bool Cubemap::addTexture(ID3D11Device* dev, std::string filepath) {
  if (filepath.empty()) {
    return false;
  }
  std::unique_ptr<Texture> text = std::make_unique<Texture>();
  text->LoadCubemapTexture(dev, filepath);
  textures_.push_back(std::move(text));
  return true;
}
#endif



Cubemap::~Cubemap() {
  for (size_t i = 0; i < textures_.size(); i++) {
    textures_.at(i).get()->FreeTexture();
  }
}