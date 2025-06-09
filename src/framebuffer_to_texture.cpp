#include "framebuffer_to_texture.hpp"

#ifdef RENDER_OPENGL
FrameBufferToTexture::FrameBufferToTexture(unsigned int dx, unsigned int dy){
  //Generate the frame buffer
  glGenFramebuffers(1, &frame_buffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

  //Generate and bind the texture buffer
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  
  //Generate the texture and set it's parameters
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA,GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id_, 0);

  loaded_ = true;
  dimensions_x = dx;
  dimensions_y = dy;
}

FrameBufferToTexture::~FrameBufferToTexture() {
  if (loaded_ && texture_id_ != -1 && frame_buffer_id_ != -1) {
    glDeleteTextures(1, &texture_id_);
    glDeleteBuffers(1, &frame_buffer_id_);
  }
}

void FrameBufferToTexture::SetBuffer() {
  if (loaded_) {glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);}
}

void FrameBufferToTexture::UnsetBuffer() {
  if (loaded_) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
}

#endif