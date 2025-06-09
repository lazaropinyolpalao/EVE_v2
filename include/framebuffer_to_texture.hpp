#ifndef __FRAME_BUFFER_TO_TEXTURE_HPP__
#define __FRAME_BUFFER_TO_TEXTURE_HPP__ 1

#ifdef RENDER_OPENGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#endif

struct FrameBufferToTexture {

  unsigned int dimensions_x;
  unsigned int dimensions_y;
  bool loaded_;
#ifdef RENDER_OPENGL
  GLuint texture_id_;
  GLuint frame_buffer_id_;
#endif

  FrameBufferToTexture() = delete;
  FrameBufferToTexture(unsigned int dx, unsigned int dy);
  ~FrameBufferToTexture();

  void SetBuffer();
  void UnsetBuffer();

};

#endif //__FRAME_BUFFER_TO_TEXTURE_HPP__
