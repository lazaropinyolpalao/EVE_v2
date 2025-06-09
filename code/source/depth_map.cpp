#include "depth_map.hpp"

#ifdef RENDER_OPENGL
DepthMap::DepthMap() {
  dimensions_ = 0;
  loaded_ = false;
  depthMapTexture_ = -1;
  depthMapFrameBufferObject_ = -1;
  type_ = DepthMapType::kTypeQuad;
}

DepthMap::DepthMap(unsigned int resolution, DepthMapType type = DepthMapType::kTypeQuad) {
  dimensions_ = 0;
  loaded_ = false;
  depthMapTexture_ = -1;
  depthMapFrameBufferObject_ = -1;
  type_ = type;
  GenerateDepthMapBuffers(resolution);
}

DepthMap::~DepthMap() {
  if (loaded_ && depthMapTexture_ != -1 && depthMapFrameBufferObject_ != -1) {
    glDeleteTextures(1, &depthMapTexture_);
    glDeleteBuffers(1, &depthMapFrameBufferObject_);
  }
}

void DepthMap::GenerateDepthMapBuffers(unsigned int resolution) {
  dimensions_ = resolution;

  //Generate the frame buffer
  glGenFramebuffers(1, &depthMapFrameBufferObject_);

  //Generate and bind the texture buffer
  glGenTextures(1, &depthMapTexture_);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFrameBufferObject_);
  if (type_ == DepthMapType::kTypeQuad) {
    glBindTexture(GL_TEXTURE_2D, depthMapTexture_);

    //Generate the texture and set it's parameters
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, dimensions_, dimensions_, 0, GL_DEPTH_COMPONENT, 
      GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  }
  else if (type_ == DepthMapType::kTypeCubeMap) { 
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMapTexture_); 

    //Generate the cube texture and set it's parameters
    for (unsigned int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
        dimensions_, dimensions_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }


  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapTexture_, 0);
  glDrawBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  loaded_ = true;

}

void DepthMap::SetBuffer() {
  if (loaded_) {
    glViewport(0, 0, dimensions_, dimensions_);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFrameBufferObject_);
  }
}

void DepthMap::UnsetBuffer(){
  if (loaded_) {glBindFramebuffer(GL_FRAMEBUFFER, 0);}
}


#endif