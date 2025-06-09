#ifndef __RENDER_SYSTEM_H__
#define __RENDER_SYSTEM_H__ 1

#include <window.hpp>
#include <resources.hpp>
#include <component_system.hpp>

class RenderSystem {
public:
  //RenderSystem() = delete;
  //~RenderSystem() = delete;

  virtual void Render(ComponentManager* comp) = 0;
  virtual void Update() = 0;
  Resources resource_list_;
  Lights lights_;
  bool IsReady_;
private:
};

#endif //__RENDER_SYSTEM_H__
