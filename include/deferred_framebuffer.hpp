#ifndef __DEFERRED_FRAMEBUFFER_HPP__
#define __DEFERRED_FRAMEBUFFER_HPP__ 1

#ifdef RENDER_OPENGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif
#include <iostream>

struct DeferredFramebuffer {

  unsigned int dimensions_x;
  unsigned int dimensions_y;
  bool loaded_;

#ifdef RENDER_OPENGL
  GLuint framebuffer_id_;

  GLuint position_texture_;
  GLuint normal_texture_;
  GLuint albedospec_texture_;

  GLuint virtual_buff_obj;
  GLuint index_buffer_object_;
  GLuint vertex_array_obj;

#endif
  DeferredFramebuffer() = delete;
  DeferredFramebuffer(unsigned int dx, unsigned int dy);
  ~DeferredFramebuffer();

  void SetBuffer();
  void UnsetBuffer();
  void RenderDeferredToQuad();

};

#endif //__DEFERRED_FRAMEBUFFER_HPP__
