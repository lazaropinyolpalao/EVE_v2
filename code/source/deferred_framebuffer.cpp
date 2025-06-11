#include "deferred_framebuffer.hpp"

#ifdef RENDER_OPENGL
DeferredFramebuffer::DeferredFramebuffer(unsigned int dx, unsigned int dy){

  // Configure gBuffer framebuffer
  glGenFramebuffers(1, &framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);

  // Configure Position color Buffer
  glGenTextures(1, &position_texture_);
  glBindTexture(GL_TEXTURE_2D, position_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dx, dy, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_texture_, 0);

  // Configure Normal color Buffer
  glGenTextures(1, &normal_texture_);
  glBindTexture(GL_TEXTURE_2D, normal_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dx, dy, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture_, 0);

  // Configure Albedo + Specular color Buffer
  glGenTextures(1, &albedospec_texture_);
  glBindTexture(GL_TEXTURE_2D, albedospec_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedospec_texture_, 0);

  // Tell opengl which color attachments we will use for rendering
  unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);

  // Create and attach depth buffer (renderbuffer)
  glGenRenderbuffers(1, &render_buffer_object_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dx, dy);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object_depth);

  // Check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Deffered Rendering Framebuffer not complete!");
    loaded_ = false;
    return ;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //Generate the quad used to render the deferred texture at fullscreen

  float vertices_[] = {
           1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
           1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
          -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
  };

  GLuint indexes_[] = {
      0, 1, 3,
      1, 2, 3
  };

  //Generate the virtual buffer, bind it, and allocate it
  glGenBuffers(1, &virtual_buff_obj);
  glBindBuffer(GL_ARRAY_BUFFER, virtual_buff_obj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_STATIC_DRAW);

  //Generate the index buffer
  glGenBuffers(1, &index_buffer_object_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes_), indexes_, GL_STATIC_DRAW);

  //Generate the vertex array
  glGenVertexArrays(1, &vertex_array_obj);
  glBindVertexArray(vertex_array_obj);

  //Enable the vertex attributes for positions, and UV
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  loaded_ = true;
  dimensions_x = dx;
  dimensions_y = dy;
}

DeferredFramebuffer::~DeferredFramebuffer() {
  if (loaded_ && framebuffer_id_ != -1) {
    glDeleteTextures(1, &position_texture_);
    glDeleteTextures(1, &normal_texture_);
    glDeleteTextures(1, &albedospec_texture_);
    glDeleteBuffers(1, &framebuffer_id_);
    glDeleteBuffers(1, &render_buffer_object_depth);
  }
}

void DeferredFramebuffer::SetBuffer() {
  if (loaded_) { glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_); }
}

void DeferredFramebuffer::UnsetBuffer() {
  if (loaded_) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
}

void DeferredFramebuffer::ResizeBuffer(int width, int height){

    dimensions_x = width;
    dimensions_y = height;

    //glDeleteTextures(1, &position_texture_);
    //glDeleteTextures(1, &normal_texture_);
    //glDeleteTextures(1, &albedospec_texture_);
    /*position_texture_ = 0;
    normal_texture_ = 0;
    albedospec_texture_ = 0;*/


    //Remake the framebuffer
    glDeleteBuffers(1, &framebuffer_id_);
    framebuffer_id_ = 0;
    glDeleteBuffers(1, &render_buffer_object_depth);
    render_buffer_object_depth = 0;

    glGenFramebuffers(1, &framebuffer_id_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);


    //Position texture
    //glGenTextures(1, &position_texture_);
    glBindTexture(GL_TEXTURE_2D, position_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dimensions_x, dimensions_y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_texture_, 0);

    // Configure Normal color Buffer
    /*glGenTextures(1, &normal_texture_);*/
    glBindTexture(GL_TEXTURE_2D, normal_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dimensions_x, dimensions_y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture_, 0);

    // Configure Albedo + Specular color Buffer
    /*glGenTextures(1, &albedospec_texture_);*/
    glBindTexture(GL_TEXTURE_2D, albedospec_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions_x, dimensions_y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedospec_texture_, 0);

    // Tell opengl which color attachments we will use for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    ////Delete previos depth buffer and recreate it with the new texture
    glGenRenderbuffers(1, &render_buffer_object_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dimensions_x, dimensions_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredFramebuffer::RenderDeferredToQuad(){

  glBindBuffer(GL_ARRAY_BUFFER, virtual_buff_obj);
  glBindVertexArray(vertex_array_obj);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object_);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

#endif