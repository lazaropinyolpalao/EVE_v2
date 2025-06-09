#ifndef __CUBEMAP_HPP__
#define __CUBEMAP_HPP__ 1


#include <vector>
#include <string>
#include <texture.hpp>

#include <defines.hpp>

#ifdef RENDER_OPENGL
  #include <GL/glew.h>
  #include <GLFW/glfw3.h>
  #include <program.hpp>
  #include <shader.hpp>
#endif
#ifdef RENDER_DIRECTX11
  
#endif

struct TinyObj;

struct Cubemap {

  int width_;
  int height_;
  int channels_;
  bool visible_;
  int selected_texture_;
  std::string path_;
  std::vector<std::unique_ptr<Texture>> textures_;
  std::unique_ptr<TinyObj> mesh_;

#ifdef RENDER_OPENGL
  std::unique_ptr<Program> program_;
  Cubemap();
  bool addTexture(std::string filepath);
#endif


#ifdef RENDER_DIRECTX11
  Cubemap(ID3D11Device* dev);
  bool addTexture(ID3D11Device* dev, std::string filepath);
#endif


  ~Cubemap();


};

#endif // __CUBEMAP_HPP__
