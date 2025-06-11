#include "engine.hpp"
#include "engine.hpp"

#define GLM_ENABLE_EXPERIMENTAL


int Engine::startUp() {
#ifdef RENDER_OPENGL
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }
#endif

  return 0;
}

int Engine::shutdown() {
#ifdef RENDER_OPENGL
  //Close glfw
  glfwTerminate();
#endif
  return 0;
}

void Engine::GameLoop(){}

Engine::Engine(int window_w, int window_h) {
  srand((unsigned int)time(NULL));
 
  component_manager_ = std::make_unique<ComponentManager>();
  scene_manager_ = std::make_unique<SceneManager>();
  boss_system_ = std::make_unique<Boss>();

  audio_device_ = alcOpenDevice("openal-soft");
  audio_context_ = alcCreateContext(audio_device_, NULL);
  alcMakeContextCurrent(audio_context_);

#ifdef RENDER_OPENGL
  render_system_ = std::make_unique<RenderSystemOpenGL>(window_w, window_h);
#endif
#ifdef RENDER_DIRECTX11
  render_system_ = std::make_unique<RenderSystemDirectX11>(window_w, window_h);
#endif

  
}

#ifdef RENDER_DIRECTX11
Engine::Engine(ID3D11Device* device, ID3D11DeviceContext* devCont,
    IDXGISwapChain* swap, ID3D11DepthStencilView* depthStencil, HWND hwnd, int window_w, int window_h){
    srand((unsigned int)time(NULL));


    component_manager_ = std::make_unique<ComponentManager>();
    scene_manager_ = std::make_unique<SceneManager>();

    render_system_ = std::make_unique<RenderSystemDirectX11>(device, devCont, swap, depthStencil, hwnd, window_w, window_h);

    printf("No render created\n");
}
#endif

Engine::~Engine() {
#ifdef RENDER_OPENGL
  glfwTerminate();
#endif

  alcMakeContextCurrent(NULL);
  alcDestroyContext(audio_context_);
  alcCloseDevice(audio_device_);

}

Window* Engine::getWindow(){
#ifdef RENDER_OPENGL
  return static_cast<RenderSystemOpenGL*>(render_system_.get())->getWindow();
#endif
#ifdef RENDER_DIRECTX11
  return static_cast<RenderSystemDirectX11*>(render_system_.get())->getWindow();
#endif

  return nullptr;
}

ComponentManager* Engine::getComponentManager(){return component_manager_.get(); }

SceneManager* Engine::getSceneManager(){return scene_manager_.get();}

RenderSystem* Engine::getRenderSystem(){return render_system_.get();}

Boss* Engine::getBossSystem(){return boss_system_.get();}

std::shared_ptr<TinyObj> Engine::AddMesh(std::string filepath){
  
#ifdef RENDER_OPENGL
  return static_cast<RenderSystemOpenGL*>(render_system_.get())->resource_list_.addMesh(filepath);
#endif
#ifdef RENDER_DIRECTX11
  RenderSystemDirectX11* r= static_cast<RenderSystemDirectX11*>(render_system_.get());
  return r->resource_list_.addMesh(r->getDevice(), r->getDeviceContext(), filepath);
#endif

  
}

void Engine::AddTexture(std::string filepath){
#ifdef RENDER_OPENGL
  static_cast<RenderSystemOpenGL*>(render_system_.get())->resource_list_.addTexture(filepath);
#endif
#ifdef RENDER_DIRECTX11
  RenderSystemDirectX11* r = static_cast<RenderSystemDirectX11*>(render_system_.get());
  r->resource_list_.addTexture(r->getDevice(), filepath);
#endif
}



void Engine::Update(){
  //Update the component manager needed elements
  component_manager_->Update();


  render_system_->Update();

  //Display Imgui functions
#ifdef RENDER_DIRECTX11
  RenderSystemDirectX11* r = static_cast<RenderSystemDirectX11*>(render_system_.get());
  //if (!r->is_uwp_) {
    ImguiFunctions::ImguiFunctionCalls(component_manager_.get(), render_system_.get(), scene_manager_.get());

  //}
#endif
#ifdef RENDER_OPENGL
  ImguiFunctions::ImguiFunctionCalls(component_manager_.get(), render_system_.get(), scene_manager_.get());
#endif 


}
 
void Engine::Render(){
  //Render the scene
  render_system_->Render(getComponentManager());
}
