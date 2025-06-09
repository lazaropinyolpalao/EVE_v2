#include <render_system_directx11.hpp>

#include <fstream>

#ifdef RENDER_DIRECTX11
#include "imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#endif

#ifdef RENDER_DIRECTX11


char* ReadShad(const char* src) {
    FILE* f;
    fopen_s(&f, src, "r");
    if (f == nullptr) { return nullptr; }
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

RenderSystemDirectX11::RenderSystemDirectX11(int window_w, int window_h){
  //## INIT THE RENDERING WINDOW
  window_ = Window::create_directx11(window_w, window_h, "Ventana", { 0.5f, 0.5f, 0.5f, 1.0f });
  if (window_ == nullptr) {
    printf("Couldn't init window");
    exit(EXIT_FAILURE);
  }


  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)window_w, (float)window_h);

  InitDirectXResources();

  // Setup Platform/RendererComponent bindings
  ImGui_ImplDX11_Init(g_pd3dDevice.Get(), g_pd3dDeviceContext.Get());
  ImGui_ImplWin32_Init(window_->window);
  
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();


  IsReady_ = true;

  //resource_list_.InitResources(getDevice());
}

RenderSystemDirectX11::RenderSystemDirectX11(ID3D11Device* device, ID3D11DeviceContext* devCont,
    IDXGISwapChain* swap, ID3D11DepthStencilView* depthStencil, HWND hwnd, int window_w, int window_h){

    window_ = std::make_unique<Window>();

    window_->clear_color_ = { 0.5f, 0.5f, 0.5f, 1.0f };
    window_->width_ = window_w;
    window_->height_ = window_h;

    window_->window = GetForegroundWindow();
    window_->marked_for_close_ = false;

    g_pd3dDevice = device;
    g_pd3dDeviceContext = devCont;
    g_pSwapChain = swap;
    g_pDSView = depthStencil;

    // display the window on the screen
    //ShowWindow(win, SW_SHOW);

    //Stablish values for the update with limited frames
    window_->refresh_rate_ = window_->get_refresh_rate();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)window_w, (float)window_h);

    //InitDirectXResources();

    is_uwp_ = true;

    // Setup Platform/RendererComponent bindings
    ImGui_ImplDX11_Init(g_pd3dDevice.Get(), g_pd3dDeviceContext.Get());
    if (window_->window != NULL) {
        ImGui_ImplWin32_Init(window_->window);
    }

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();


    InitDirectXResourcesWithWindow();

    IsReady_ = true;
}

RenderSystemDirectX11::~RenderSystemDirectX11(){}

void RenderSystemDirectX11::Render(ComponentManager* comp){
  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();

  size_t transform_iterator = 0;


  // #### RENDER ####
  CameraComponent* c = comp->get_principal_camera();
  PerFrame _updatedBuffer;
  _updatedBuffer.projection = c->projection_;
  _updatedBuffer.view = c->view_;

  ObjectLightInteraction o;
  o.viewPos = c->position_;

  g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  unsigned int stride = sizeof(Vertex);
  unsigned int offset = 0;


  bool first_time = true;

  //Directionals
  g_pd3dDeviceContext->VSSetShader(directionalLightVertexShader.Get(), nullptr, 0);
  g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, g_pTransProjViewConstantBuffer.GetAddressOf());
  g_pd3dDeviceContext->PSSetShader(directionalLightPixelShader.Get(), nullptr, 0);
  g_pd3dDeviceContext->PSSetConstantBuffers(0, 1, objectLightInteractionConstantBuffer.GetAddressOf());
  g_pd3dDeviceContext->PSSetConstantBuffers(1, 1, directionalLightUniformsConstantBuffer.GetAddressOf());

  transform_iterator = 0;
  for (unsigned int i = 0; i < lights_.directional_.size();++i) {
      DirectionalLight *dir = lights_.directional_.at(i).get();
      if (dir->visible_) {
          DirectionalLightUniforms d;
          d.directionalLightMatrix = dir->lightMatrix_;
          d.directionalPosition = dir->position_;
          d.directionalDirection = dir->direction_;
          d.directionalAmbient = dir->ambient_;
          d.directionalDiffuse = dir->diffuse_;
          d.directionalSpecular = dir->specular_;

          //Update the light constant buffer
          g_pd3dDeviceContext->UpdateSubresource(directionalLightUniformsConstantBuffer.Get(), 0, NULL, &d, 0, 0);

          //Set blend state to no for the first time
          if (first_time) {
              g_pd3dDeviceContext->OMSetBlendState(g_BlendStateNoBlend.Get(), 0, 0xffffffff);
          }

          for (unsigned int it = 0; it < render_size; ++it) {
              size_t id = renderer_components->at(it).entity_id_;
              RendererComponent* r = &(renderer_components->at(it).data_);
              TransformComponent* t = nullptr;

              //Search for the assigned transform of this renderer if it exists
              while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
                  transform_iterator++;
              }

              if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
                  t = &(transform_components->at(transform_iterator).data_);
              }

              if (r->isInit_ && r->mesh_->isInit_) {
                  glm::mat4 trans = glm::mat4(1.0f);
                  if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }
                  //RenderObject(comp, t, r);

                  //Update constant buffers
                  _updatedBuffer.transform = trans;
                  g_pd3dDeviceContext->UpdateSubresource(g_pTransProjViewConstantBuffer.Get(), 0, NULL, &_updatedBuffer, 0, 0);

                  o.receiveShadows = r->receives_shadows_;
                  o.needsLight = r->needs_light_;

                  g_pd3dDeviceContext->UpdateSubresource(objectLightInteractionConstantBuffer.Get(), 0, NULL, &o, 0, 0);


                  // Set array of vertices and indices
                  g_pd3dDeviceContext->IASetVertexBuffers(0, 1, r->mesh_->g_pVertexBuffer.GetAddressOf(), &stride, &offset);
                  g_pd3dDeviceContext->IASetIndexBuffer(r->mesh_->g_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

                  if (r->textures_.size() != 0) {
                      //Textures
                      g_pd3dDeviceContext->PSSetShaderResources(0, 1, r->textures_.at(0)->shader_resource_view_.GetAddressOf()); // Bind SRV to slot 0
                      g_pd3dDeviceContext->PSSetSamplers(0, 1, r->textures_.at(0)->sampler_.GetAddressOf());
                  }

                  //Draw the mesh
                  g_pd3dDeviceContext->DrawIndexed((UINT)r->mesh_->indexes_.size(), 0, 0);
              }
          }
      
      
          // Set blend state to yes for the rest of the render
          if (first_time) {
              g_pd3dDeviceContext->OMSetBlendState(g_BlendStateBlend.Get(), 0, 0xffffffff);
              first_time = false;
          }
      }
  }

  //Spots
  g_pd3dDeviceContext->VSSetShader(spotLightVertexShader.Get(), nullptr, 0);
  g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, g_pTransProjViewConstantBuffer.GetAddressOf());
  g_pd3dDeviceContext->PSSetShader(spotLightPixelShader.Get(), nullptr, 0);
  g_pd3dDeviceContext->PSSetConstantBuffers(0, 1, objectLightInteractionConstantBuffer.GetAddressOf());
  g_pd3dDeviceContext->PSSetConstantBuffers(1, 1, spotLightUniformsConstantBuffer.GetAddressOf());

  transform_iterator = 0;
  for (unsigned int i = 0; i < lights_.spot_.size(); ++i) {
      SpotLight* spot = lights_.spot_.at(i).get();
      if (spot->visible_) {
          SpotLightUniforms s;
          s.position = spot->position_;
          s.direction = spot->direction_;
          s.diffuse = spot->diffuse_;
          s.specular = spot->specular_;
          s.cutOff = spot->cut_off_;
          s.outerCutOff = spot->outer_cut_off_;
          s.constant = spot->constant_;
          s.linear_light = spot->linear_;
          s.quadratic = spot->quadratic_;

          //Update the light constant buffer
          g_pd3dDeviceContext->UpdateSubresource(spotLightUniformsConstantBuffer.Get(), 0, NULL, &s, 0, 0);

          //Set blend state to no for the first time
          if (first_time) {
              g_pd3dDeviceContext->OMSetBlendState(g_BlendStateNoBlend.Get(), 0, 0xffffffff);
          }

          for (unsigned int it = 0; it < render_size; ++it) {
              size_t id = renderer_components->at(it).entity_id_;
              RendererComponent* r = &(renderer_components->at(it).data_);
              TransformComponent* t = nullptr;

              //Search for the assigned transform of this renderer if it exists
              while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
                  transform_iterator++;
              }

              if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
                  t = &(transform_components->at(transform_iterator).data_);
              }

              if (r->isInit_ && r->mesh_->isInit_) {
                  glm::mat4 trans = glm::mat4(1.0f);
                  if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }
                  //RenderObject(comp, t, r);

                  //Update constant buffers
                  _updatedBuffer.transform = trans;
                  g_pd3dDeviceContext->UpdateSubresource(g_pTransProjViewConstantBuffer.Get(), 0, NULL, &_updatedBuffer, 0, 0);

                  o.receiveShadows = r->receives_shadows_;
                  o.needsLight = r->needs_light_;

                  g_pd3dDeviceContext->UpdateSubresource(objectLightInteractionConstantBuffer.Get(), 0, NULL, &o, 0, 0);


                  // Set array of vertices and indices
                  g_pd3dDeviceContext->IASetVertexBuffers(0, 1, r->mesh_->g_pVertexBuffer.GetAddressOf(), &stride, &offset);
                  g_pd3dDeviceContext->IASetIndexBuffer(r->mesh_->g_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

                  if (r->textures_.size() != 0) {
                      //Textures
                      g_pd3dDeviceContext->PSSetShaderResources(0, 1, r->textures_.at(0)->shader_resource_view_.GetAddressOf()); // Bind SRV to slot 0
                      g_pd3dDeviceContext->PSSetSamplers(0, 1, r->textures_.at(0)->sampler_.GetAddressOf());
                  }

                  //Draw the mesh
                  g_pd3dDeviceContext->DrawIndexed((UINT)r->mesh_->indexes_.size(), 0, 0);
              }
          }


          // Set blend state to yes for the rest of the render
          if (first_time) {
              g_pd3dDeviceContext->OMSetBlendState(g_BlendStateBlend.Get(), 0, 0xffffffff);
              first_time = false;
          }
      }
  }

  window_->render_xbox(g_pd3dDeviceContext.Get(), g_mainRenderTargetView.Get(), g_pSwapChain.Get(), g_pDSView.Get(), is_uwp_);

}

void RenderSystemDirectX11::Update(){
  //Update

    if (!is_uwp_) {
        window_->clear(g_pd3dDeviceContext.Get(), g_mainRenderTargetView.Get(), g_pDSView.Get());
    }
    else {
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();
    }

    window_->update_delta();
    window_->detect_events();
}

Window* RenderSystemDirectX11::getWindow(){return window_.get();}
ID3D11Device* RenderSystemDirectX11::getDevice(){return g_pd3dDevice.Get();}
ID3D11DeviceContext* RenderSystemDirectX11::getDeviceContext(){return g_pd3dDeviceContext.Get();}

bool RenderSystemDirectX11::InitDirectXResources(){

    HRESULT hr;

    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = window_->width_;
    sd.BufferDesc.Height = window_->height_;
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = window_->refresh_rate_;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window_->window;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
      // TODO: Modify for supported Direct3D feature levels
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
    };
  
    //Create device and swap chain
    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 
        D3D11_CREATE_DEVICE_DEBUG, featureLevels, 2, D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pd3dDeviceContext);

    if(g_pSwapChain == nullptr || g_pd3dDevice == nullptr){return false;}

    //Create render target
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &g_BackBuffer);
    g_pd3dDevice->CreateRenderTargetView(g_BackBuffer.Get(), nullptr, g_mainRenderTargetView.GetAddressOf());

    //Set the viewport
    D3D11_VIEWPORT viewport = { 0 };
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)window_->width_;
    viewport.Height = (float)window_->height_;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    g_pd3dDeviceContext->RSSetViewports(1, &viewport);


    //Create depth stencil and initial depth testing values
    //*/
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = window_->width_;
    descDepth.Height = window_->height_;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, g_pDepthStencil.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating texture for depth: %#010x\n", hr); }

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil.Get(), &descDSV, g_pDSView.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating depth stencil view: %#010x\n", hr); }
    /**/
    // Depth test parameters
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create depth stencil state
    hr = g_pd3dDevice->CreateDepthStencilState(&dsDesc, g_pDSState.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating depth stencil: %#010x\n", hr); }
    g_pd3dDeviceContext->OMSetDepthStencilState(g_pDSState.Get(), 1);
    /**/

    //Rasterizer
    /**/
    D3D11_RASTERIZER_DESC rasterizer_state_description = { };
    rasterizer_state_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_state_description.CullMode = D3D11_CULL_BACK;
    rasterizer_state_description.FrontCounterClockwise = true;
    rasterizer_state_description.DepthBias = 0;
    rasterizer_state_description.DepthBiasClamp = 0.0f;
    rasterizer_state_description.SlopeScaledDepthBias = 0;
    rasterizer_state_description.DepthClipEnable = true;
    rasterizer_state_description.ScissorEnable = false;
    rasterizer_state_description.MultisampleEnable = false;
    rasterizer_state_description.AntialiasedLineEnable = false;
    hr = g_pd3dDevice->CreateRasterizerState(&rasterizer_state_description, g_pRasterizerState.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating rasterizer state: %#010x\n", hr); }
    g_pd3dDeviceContext->RSSetState(g_pRasterizerState.Get());
    /**/

    //Blend state without blend
    D3D11_BLEND_DESC blend_description = {};
    blend_description.RenderTarget[0].BlendEnable = false;
    blend_description.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blend_description.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blend_description.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_description.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_description.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState(&blend_description, g_BlendStateNoBlend.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating blend state no blend: %#010x\n", hr); }

    //Blend state with blend

    D3D11_BLEND_DESC blend_description_blend = {};
    blend_description_blend.RenderTarget[0].BlendEnable = true;
    blend_description_blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blend_description_blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blend_description_blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_description_blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_description_blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blend_description_blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_description_blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState(&blend_description, g_BlendStateBlend.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating blend state with blend: %#010x\n", hr); }

    g_pd3dDeviceContext->OMSetBlendState(g_BlendStateNoBlend.Get(), 0, 0xffffffff);
    /**/
    /**/

    InitShaders();

    return true;

}

bool RenderSystemDirectX11::InitDirectXResourcesWithWindow(){
    
    //TODO: Init components
    //Get depth stencil view

    D3D11_RASTERIZER_DESC rasterizer_state_description = { };
    rasterizer_state_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_state_description.CullMode = D3D11_CULL_BACK;
    rasterizer_state_description.FrontCounterClockwise = true;
    rasterizer_state_description.DepthBias = 0;
    rasterizer_state_description.DepthBiasClamp = 0.0f;
    rasterizer_state_description.SlopeScaledDepthBias = 0;
    rasterizer_state_description.DepthClipEnable = true;
    rasterizer_state_description.ScissorEnable = false;
    rasterizer_state_description.MultisampleEnable = false;
    rasterizer_state_description.AntialiasedLineEnable = false;
    HRESULT hr = g_pd3dDevice->CreateRasterizerState(&rasterizer_state_description, g_pRasterizerState.GetAddressOf());
    if (FAILED(hr)) { printf("Error creating rasterizer state: %#010x\n", hr); }
    g_pd3dDeviceContext->RSSetState(g_pRasterizerState.Get());

    InitShadersUWP();

    return false;
}

bool RenderSystemDirectX11::InitShadersUWP(){ 
    // #### PREPARE SHADERS ####

    // Create Default Vertex Shader
    std::ifstream ifsv("Assets/default_vertex_shader.shader");
    if (ifsv.fail()) {
        printf("Default vertex shader not found\n");
        return false;
    }
    std::string contentv((std::istreambuf_iterator<char>(ifsv)), (std::istreambuf_iterator<char>()));


    ID3DBlob* vertexShaderBlob = nullptr;
    HRESULT vertexCompile = D3DCompile(contentv.c_str(), contentv.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &vertexShaderBlob, nullptr);
    if (FAILED(vertexCompile)) {
        printf("Test vertex shader error: %#010x\n", vertexCompile);
        return false;
    }

    // Create Input Layout
    D3D11_INPUT_ELEMENT_DESC iaDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    g_pd3dDevice->CreateInputLayout(iaDesc, ARRAYSIZE(iaDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), g_pInputLayout.GetAddressOf());
    g_pd3dDeviceContext->IASetInputLayout(g_pInputLayout.Get());

    HRESULT hrVertex = g_pd3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, g_pVertexShader.GetAddressOf());
    if (FAILED(hrVertex)) { 
        printf("Vertex Shader creation failed: %#010x\n", hrVertex); 
        return false;
    }
    vertexShaderBlob->Release();

    // Create Default Pixel Shader  
    //*/
    std::ifstream ifsp("Assets/default_pixel_shader_color.shader");
    if (ifsp.fail()) {
        printf("Default vertex shader not found\n");
        return false;
    }
    std::string contentp((std::istreambuf_iterator<char>(ifsp)), (std::istreambuf_iterator<char>()));
    /*/
    std::string contentp;
    t = ReadShad("Assets/default_pixel_shader_color.shader");
    contentp.append(t);
    //free(t);
    /**/

    ID3DBlob* pixelShaderBlob = nullptr;
    HRESULT pixelCompile = D3DCompile(contentp.c_str(), contentp.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &pixelShaderBlob, nullptr);
    if (FAILED(pixelCompile)) {
        printf("Test pixel shader error: %#010x\n", pixelCompile);
        return false;
    }

    HRESULT hrPixel = g_pd3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, g_pPixelShader.GetAddressOf());
    if (FAILED(hrPixel)) { 
        printf("Pixel Shader creation failed: %#010x\n", hrPixel); 
        return false;
    }
    pixelShaderBlob->Release();




    // Constant Buffer
    PerFrame _updatedBuffer;
    _updatedBuffer.transform = glm::mat4(1.0);
    _updatedBuffer.projection = glm::mat4(1.0);
    _updatedBuffer.view = glm::mat4(1.0);

    CD3D11_BUFFER_DESC constantBufferDesc(sizeof(PerFrame), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrConstBuff = g_pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_pTransProjViewConstantBuffer);
    if (FAILED(hrConstBuff)) {
        printf("Error creating constant buffer: %#010x\n", hrConstBuff);
        return false;
    }

    //### LIGHT SHADERS

    //--DIRECTIONAL
    // Create Directional Light Vertex Shader 
    std::ifstream ifsDirectionalVS("../data/shaders_directx11/forward_rendering/directional_elements_vertex_shader.hlsl");
    if (ifsDirectionalVS.fail()) {
        printf("Default vertex shader not found\n");
        return false;
    }
    std::string contentDirectionalVS((std::istreambuf_iterator<char>(ifsDirectionalVS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* directionalVSBlob = nullptr;
    HRESULT directionalVSCompile = D3DCompile(contentDirectionalVS.c_str(), contentDirectionalVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &directionalVSBlob, nullptr);
    if (FAILED(directionalVSCompile)) {
        printf("Directional Light Vertex Shader Compilation Error: %#010x\n", directionalVSCompile);
        return false;
    }

    HRESULT hrDirectionalVS = g_pd3dDevice->CreateVertexShader(directionalVSBlob->GetBufferPointer(), directionalVSBlob->GetBufferSize(), nullptr, directionalLightVertexShader.GetAddressOf());
    if (FAILED(hrDirectionalVS)) { printf("Directional Light Vertex Shader Creation Failed: %#010x\n", hrDirectionalVS); }

    directionalVSBlob->Release();


    // Create Directional Light Pixel Shader
    std::ifstream ifsDirectionalPS("../data/shaders_directx11/forward_rendering/directional_elements_pixel_shader.hlsl");
    if (ifsDirectionalPS.fail()) {
        printf("Directional vertex shader not found\n");
        return true;
    }
    std::string contentDirectionalPS((std::istreambuf_iterator<char>(ifsDirectionalPS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* directionalPSBlob = nullptr;
    HRESULT directionalPSCompile = D3DCompile(contentDirectionalPS.c_str(), contentDirectionalPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &directionalPSBlob, nullptr);
    if (FAILED(directionalPSCompile)) {
        printf("Directional Light Pixel Shader Compilation Error: %#010x\n", directionalPSCompile);
        return false;
    }

    HRESULT hrDirectionalPS = g_pd3dDevice->CreatePixelShader(directionalPSBlob->GetBufferPointer(), directionalPSBlob->GetBufferSize(), nullptr, directionalLightPixelShader.GetAddressOf());
    if (FAILED(hrDirectionalPS)) { printf("Directional Light Pixel Shader Creation Failed: %#010x\n", hrDirectionalPS); }
    directionalPSBlob->Release();


    //--SPOT

    // Create Spot Light Vertex Shader 
    std::ifstream ifsSpotVS("shaders_directx11/forward_rendering/spotlight_elements_vertex_shader.hlsl");
    if (ifsSpotVS.fail()) {
        printf("Default vertex shader not found\n");
        return false;
    }
    std::string contentSpotVS((std::istreambuf_iterator<char>(ifsSpotVS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* spotVSBlob = nullptr;
    HRESULT spotVSCompile = D3DCompile(contentSpotVS.c_str(), contentSpotVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &spotVSBlob, nullptr);
    if (FAILED(spotVSCompile)) {
        printf("Spot Light Vertex Shader Compilation Error: %#010x\n", spotVSCompile);
        return false;
    }

    HRESULT hrSpotVS = g_pd3dDevice->CreateVertexShader(spotVSBlob->GetBufferPointer(), spotVSBlob->GetBufferSize(), nullptr, spotLightVertexShader.GetAddressOf());
    if (FAILED(hrSpotVS)) { printf("Directional Light Vertex Shader Creation Failed: %#010x\n", hrSpotVS); }

    spotVSBlob->Release();


    // Create Spot Light Pixel Shader
    std::ifstream ifsSpotPS("../data/shaders_directx11/forward_rendering/spotlight_elements_pixel_shader.hlsl");
    if (ifsSpotPS.fail()) {
        printf("Spot vertex shader not found\n");
        return true;
    }
    std::string contentSpotPS((std::istreambuf_iterator<char>(ifsSpotPS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* spotPSBlob = nullptr;
    HRESULT spotPSCompile = D3DCompile(contentSpotPS.c_str(), contentSpotPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &spotPSBlob, nullptr);
    if (FAILED(spotPSCompile)) {
        printf("Spot Light Pixel Shader Compilation Error: %#010x\n", spotPSCompile);
        return false;
    }

    HRESULT hrSpotPS = g_pd3dDevice->CreatePixelShader(spotPSBlob->GetBufferPointer(), spotPSBlob->GetBufferSize(), nullptr, spotLightPixelShader.GetAddressOf());
    if (FAILED(hrSpotPS)) { 
        printf("Spot Light Pixel Shader Creation Failed: %#010x\n", hrSpotPS); 
        return false;
    }
    spotPSBlob->Release();



    //--POINT

    std::ifstream ifsPointVS("shaders_directx11/forward_rendering/pointlight_elements_vertex_shader.hlsl");
    if (ifsPointVS.fail()) {
        printf("Default vertex shader not found\n");
        return false;
    }
    std::string contentPointVS((std::istreambuf_iterator<char>(ifsPointVS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* pointVSBlob = nullptr;
    HRESULT pointVSCompile = D3DCompile(contentPointVS.c_str(), contentPointVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &pointVSBlob, nullptr);
    if (FAILED(pointVSCompile)) {
        printf("Point Light Vertex Shader Compilation Error: %#010x\n", pointVSCompile);
        return false;
    }

    HRESULT hrPointVS = g_pd3dDevice->CreateVertexShader(pointVSBlob->GetBufferPointer(), pointVSBlob->GetBufferSize(), nullptr, pointLightVertexShader.GetAddressOf());
    if (FAILED(hrPointVS)) { printf("Point Light Vertex Shader Creation Failed: %#010x\n", hrPointVS); }

    pointVSBlob->Release();


    // Create Point Light Pixel Shader
    std::ifstream ifsPointPS("../data/shaders_directx11/forward_rendering/pointlight_elements_pixel_shader.hlsl");
    if (ifsPointPS.fail()) {
        printf("Point light vertex shader not found\n");
        return true;
    }
    std::string contentPointPS((std::istreambuf_iterator<char>(ifsPointPS)), (std::istreambuf_iterator<char>()));

    ID3DBlob* pointPSBlob = nullptr;
    HRESULT pointPSCompile = D3DCompile(contentPointPS.c_str(), contentPointPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &pointPSBlob, nullptr);
    if (FAILED(pointPSCompile)) {
        printf("Point Light Pixel Shader Compilation Error: %#010x\n", pointPSCompile);
        return false;
    }

    HRESULT hrPointPS = g_pd3dDevice->CreatePixelShader(pointPSBlob->GetBufferPointer(), pointPSBlob->GetBufferSize(), nullptr, pointLightPixelShader.GetAddressOf());
    if (FAILED(hrPointPS)) { printf("Point Light Pixel Shader Creation Failed: %#010x\n", hrPointPS); }
    pointPSBlob->Release();


    //## CONSTANT BUFFERS
    // Per Object Light Interaction Constant Buffer
    ObjectLightInteraction oblConstantBuffer;
    oblConstantBuffer.viewPos = glm::vec3(0.0);
    oblConstantBuffer.receiveShadows = true;
    oblConstantBuffer.needsLight = true;

    D3D11_BUFFER_DESC oblBufferDesc;
    oblBufferDesc.ByteWidth = sizeof(ObjectLightInteraction);
    oblBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    oblBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    oblBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    oblBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA oblInitData;
    oblInitData.pSysMem = &oblConstantBuffer;
    oblInitData.SysMemPitch = 0;
    oblInitData.SysMemSlicePitch = 0;
    HRESULT hrOblConstBuff = g_pd3dDevice->CreateBuffer(&oblBufferDesc, &oblInitData, objectLightInteractionConstantBuffer.GetAddressOf());
    if (FAILED(hrOblConstBuff)) {
        printf("Error creating object constant buffer: %#010x\n", hrOblConstBuff);
        return false;
    }
    /**/

    return true;
}

bool RenderSystemDirectX11::InitShaders()
{

    D3D11_INPUT_ELEMENT_DESC iaDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // #### PREPARE SHADERS ####

    // ## DEFAULT SHADERS ##
    {
        std::ifstream ifsv("../data/shaders_directx11/default_vertex_shader.hlsl");
        if (ifsv.fail()) {
            printf("Default vertex shader not found\n");
            return false;
        }
        std::string contentv((std::istreambuf_iterator<char>(ifsv)), (std::istreambuf_iterator<char>()));


        ID3DBlob* vertexShaderBlob = nullptr;
        HRESULT vertexCompile = D3DCompile(contentv.c_str(), contentv.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &vertexShaderBlob, nullptr);
        if (FAILED(vertexCompile)) {
            printf("Test vertex shader error: %#010x\n", vertexCompile);
            return false;
        }

        // Create Input Layout
        
        g_pd3dDevice->CreateInputLayout(iaDesc, ARRAYSIZE(iaDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), g_pInputLayout.GetAddressOf());
        g_pd3dDeviceContext->IASetInputLayout(g_pInputLayout.Get());

        HRESULT hrVertex = g_pd3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, g_pVertexShader.GetAddressOf());
        if (FAILED(hrVertex)) { printf("Vertex Shader creation failed: %#010x\n", hrVertex); }
        vertexShaderBlob->Release();



        // Create Default Pixel Shader  
        std::ifstream ifsp("../data/shaders_directx11/default_pixel_shader_color.hlsl");
        if (ifsp.fail()) {
            printf("Default vertex shader not found\n");
            return true;
        }
        std::string contentp((std::istreambuf_iterator<char>(ifsp)), (std::istreambuf_iterator<char>()));

        ID3DBlob* pixelShaderBlob = nullptr;
        HRESULT pixelCompile = D3DCompile(contentp.c_str(), contentp.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &pixelShaderBlob, nullptr);
        if (FAILED(pixelCompile)) {
            printf("Test pixel shader error: %#010x\n", pixelCompile);
            //return false;
        }

        HRESULT hrPixel = g_pd3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, g_pPixelShader.GetAddressOf());
        if (FAILED(hrPixel)) { printf("Pixel Shader creation failed: %#010x\n", hrPixel); }
        pixelShaderBlob->Release();


    }

    //## DIRECTIONAL LIGHT SHADERS
    {
        // Create Directional Light Vertex Shader 
        std::ifstream ifsDirectionalVS("../data/shaders_directx11/forward_rendering/directional_elements_vertex_shader.hlsl");
        if (ifsDirectionalVS.fail()) {
            printf("Directional vertex shader not found\n");
        }
        std::string contentDirectionalVS((std::istreambuf_iterator<char>(ifsDirectionalVS)), (std::istreambuf_iterator<char>()));

        ID3DBlob* directionalVSBlob = nullptr;
        HRESULT directionalVSCompile = D3DCompile(contentDirectionalVS.c_str(), contentDirectionalVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &directionalVSBlob, nullptr);
        if (FAILED(directionalVSCompile)) {
            printf("Directional Light Vertex Shader Compilation Error: %#010x\n", directionalVSCompile);
            //return false;
        }

        HRESULT hrDirectionalVS = g_pd3dDevice->CreateVertexShader(directionalVSBlob->GetBufferPointer(), directionalVSBlob->GetBufferSize(), nullptr, directionalLightVertexShader.GetAddressOf());
        if (FAILED(hrDirectionalVS)) { printf("Directional Light Vertex Shader Creation Failed: %#010x\n", hrDirectionalVS); }

        directionalVSBlob->Release();

        // Create Directional Light Pixel Shader

        //*/
        std::ifstream ifsDirectionalPS("../data/shaders_directx11/forward_rendering/directional_elements_pixel_shader.hlsl");
        if (ifsDirectionalPS.fail()) {
            printf("Directional pixel shader not found\n");
            return false;
        }
        std::string contentDirectionalPS((std::istreambuf_iterator<char>(ifsDirectionalPS)), (std::istreambuf_iterator<char>()));
        /*/

        std::string contentDirectionalPS;
        t = ReadShad("../data/shaders_directx11/forward_rendering/directional_elements_pixel_shader.hlsl");
        contentDirectionalPS.append(t);
        free(t);
        /**/

        ID3DBlob* directionalPSBlob = nullptr;
        HRESULT directionalPSCompile = D3DCompile(contentDirectionalPS.c_str(), contentDirectionalPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &directionalPSBlob, nullptr);
        if (FAILED(directionalPSCompile)) {
            printf("Directional Light Pixel Shader Compilation Error: %#010x\n", directionalPSCompile);
            //return false;
        }

        HRESULT hrDirectionalPS = g_pd3dDevice->CreatePixelShader(directionalPSBlob->GetBufferPointer(), directionalPSBlob->GetBufferSize(), nullptr, directionalLightPixelShader.GetAddressOf());
        if (FAILED(hrDirectionalPS)) { printf("Directional Light Pixel Shader Creation Failed: %#010x\n", hrDirectionalPS); }
        directionalPSBlob->Release();
    }


    //## SPOT LIGHT SHADERS
    {
        // Create Spot Light Vertex Shader 
        std::ifstream ifsSpotVS("../data/shaders_directx11/forward_rendering/spotlight_elements_vertex_shader.hlsl");
        if (ifsSpotVS.fail()) {
            printf("Spot vertex shader not found\n");
            return false;
        }
        std::string contentSpotVS((std::istreambuf_iterator<char>(ifsSpotVS)), (std::istreambuf_iterator<char>()));

        ID3DBlob* spotVSBlob = nullptr;
        HRESULT spotVSCompile = D3DCompile(contentSpotVS.c_str(), contentSpotVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &spotVSBlob, nullptr);
        if (FAILED(spotVSCompile)) {
            printf("Spot Light Vertex Shader Compilation Error: %#010x\n", spotVSCompile);
            return false;
        }

        HRESULT hrSpotVS = g_pd3dDevice->CreateVertexShader(spotVSBlob->GetBufferPointer(), spotVSBlob->GetBufferSize(), nullptr, spotLightVertexShader.GetAddressOf());
        if (FAILED(hrSpotVS)) { 
            printf("Directional Light Vertex Shader Creation Failed: %#010x\n", hrSpotVS); 
            return false;
        }

        spotVSBlob->Release();


        // Create Spot Light Pixel Shader
        std::ifstream ifsSpotPS("../data/shaders_directx11/forward_rendering/spotlight_elements_pixel_shader.hlsl");
        if (ifsSpotPS.fail()) {
            printf("Spot pixel shader not found\n");
            return true;
        }
        std::string contentSpotPS((std::istreambuf_iterator<char>(ifsSpotPS)), (std::istreambuf_iterator<char>()));

        //printf("%s\n", contentSpotPS.c_str());

        ID3DBlob* spotPSBlob = nullptr;
        HRESULT spotPSCompile = D3DCompile(contentSpotPS.c_str(), contentSpotPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &spotPSBlob, nullptr);
        if (FAILED(spotPSCompile)) {
            printf("Spot Light Pixel Shader Compilation Error: %#010x\n", spotPSCompile);
            return false;
        }

        HRESULT hrSpotPS = g_pd3dDevice->CreatePixelShader(spotPSBlob->GetBufferPointer(), spotPSBlob->GetBufferSize(), nullptr, spotLightPixelShader.GetAddressOf());
        if (FAILED(hrSpotPS)) { 
            printf("Spot Light Pixel Shader Creation Failed: %#010x\n", hrSpotPS); 
            return false;
        }
        spotPSBlob->Release();
    }

    //## POINT LIGHT SHADERS
    {
        std::ifstream ifsPointVS("../data/shaders_directx11/forward_rendering/pointlight_elements_vertex_shader.hlsl");
        if (ifsPointVS.fail()) {
            printf("Point vertex shader not found\n");
            return false;
        }
        std::string contentPointVS((std::istreambuf_iterator<char>(ifsPointVS)), (std::istreambuf_iterator<char>()));

        ID3DBlob* pointVSBlob = nullptr;
        HRESULT pointVSCompile = D3DCompile(contentPointVS.c_str(), contentPointVS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_VERTEX_SHADER_VERSION, 0, 0, &pointVSBlob, nullptr);
        if (FAILED(pointVSCompile)) {
            printf("Point Light Vertex Shader Compilation Error: %#010x\n", pointVSCompile);
            return false;
        }

        HRESULT hrPointVS = g_pd3dDevice->CreateVertexShader(pointVSBlob->GetBufferPointer(), pointVSBlob->GetBufferSize(), nullptr, pointLightVertexShader.GetAddressOf());
        if (FAILED(hrPointVS)) { printf("Point Light Vertex Shader Creation Failed: %#010x\n", hrPointVS); }

        pointVSBlob->Release();


        // Create Point Light Pixel Shader
        std::ifstream ifsPointPS("../data/shaders_directx11/forward_rendering/pointlight_elements_pixel_shader.hlsl");
        if (ifsPointPS.fail()) {
            printf("Point light vertex shader not found\n");
            return true;
        }
        std::string contentPointPS((std::istreambuf_iterator<char>(ifsPointPS)), (std::istreambuf_iterator<char>()));

        ID3DBlob* pointPSBlob = nullptr;
        HRESULT pointPSCompile = D3DCompile(contentPointPS.c_str(), contentPointPS.length(), nullptr, nullptr, nullptr, "Main", DIRECTX_PIXEL_SHADER_VERSION, 0, 0, &pointPSBlob, nullptr);
        if (FAILED(pointPSCompile)) {
            printf("Point Light Pixel Shader Compilation Error: %#010x\n", pointPSCompile);
            return false;
        }

        HRESULT hrPointPS = g_pd3dDevice->CreatePixelShader(pointPSBlob->GetBufferPointer(), pointPSBlob->GetBufferSize(), nullptr, pointLightPixelShader.GetAddressOf());
        if (FAILED(hrPointPS)) { printf("Point Light Pixel Shader Creation Failed: %#010x\n", hrPointPS); }
        pointPSBlob->Release();
    }

    // ## CONSTANT BUFFERS

    // Default world constant buffer
    CD3D11_BUFFER_DESC constantBufferDesc(sizeof(PerFrame), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrConstBuff = g_pd3dDevice->CreateBuffer(&constantBufferDesc,nullptr,&g_pTransProjViewConstantBuffer);
    if (FAILED(hrConstBuff)) { 
        printf("Error creating constant buffer: %#010x\n", hrConstBuff);
        return false;
    }

    // Per Object Light Interaction Constant Buffer
    CD3D11_BUFFER_DESC oblBufferDesc(sizeof(ObjectLightInteraction), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrOblConstBuff = g_pd3dDevice->CreateBuffer(&oblBufferDesc, nullptr, &objectLightInteractionConstantBuffer);
    if (FAILED(hrOblConstBuff)) {
        printf("Error creating object constant buffer: %#010x\n", hrOblConstBuff);
        return false;
    }

    // ## DIRECTIONAL LIGHT CONSTANT BUFFER
    CD3D11_BUFFER_DESC directionalBufferDesc(sizeof(DirectionalLightUniforms), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrDirectionalConstBuff = g_pd3dDevice->CreateBuffer(&directionalBufferDesc, nullptr, &directionalLightUniformsConstantBuffer);
    if (FAILED(hrDirectionalConstBuff)) { 
        printf("Error creating Directional Light constant buffer: %#010x\n", hrDirectionalConstBuff); 
        return false;
    }

    // ## SPOT LIGHT CONSTANT BUFFER
    CD3D11_BUFFER_DESC spotBufferDesc(sizeof(SpotLightUniforms), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrSpotConstBuff = g_pd3dDevice->CreateBuffer(&spotBufferDesc, nullptr, &spotLightUniformsConstantBuffer);
    if (FAILED(hrSpotConstBuff)) {
        printf("Error creating Directional Light constant buffer: %#010x\n", hrSpotConstBuff);
        return false;
    }

    // ## POINT LIGHT CONSTANT BUFFER
    CD3D11_BUFFER_DESC pointBufferDesc(sizeof(PointLightUniforms), D3D11_BIND_CONSTANT_BUFFER);
    HRESULT hrPointConstBuff = g_pd3dDevice->CreateBuffer(&pointBufferDesc, nullptr, &pointLightUniformsConstantBuffer);
    if (FAILED(hrPointConstBuff)) {
        printf("Error creating Directional Light constant buffer: %#010x\n", hrPointConstBuff);
        return false;
    }

    // Set primitive render topology (pointlist, lines, triangles, etc.)
    g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}



#endif
/*/
void RenderSystemDirectX11::render_scene_cubemap(ComponentManager* comp){

  Cubemap* cubemap = resource_list_.cubemap_.get();

  if (cubemap->visible_) {

    cubemap->program_->Use();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glm::mat4 cam_projection = glm::mat4(1.0f);
    glm::mat4 cam_view = glm::mat4(1.0f);
    glm::vec3 cam_position = glm::vec3(0.0f);

    CameraComponent* cam = comp->get_principal_camera();
    if (cam != nullptr) {
      cam_projection = cam->projection_;
      cam_view = cam->view_;
      cam_position = cam->position_;
    }
    else { cam_projection = glm::perspective(90.0f, (float)(window_.get()->width_ / window_.get()->height_), 0.01f, 2000.0f); }

    cubemap->program_->SetMat4("projection", (float*)glm::value_ptr(cam_projection));
    cubemap->program_->SetMat4("view", (float*)glm::value_ptr(cam_view));
    cubemap->program_->SetVec3("viewPos", cam_position);

    glm::mat4 t = glm::mat4(1.0f);
    glm::mat4 tmp_scl = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::quat pitch = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yaw = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat roll = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0));
    glm::mat4 tmp_rot = glm::mat4_cast(yaw * pitch * roll);
    glm::mat4 tmp_pos = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    t = tmp_pos * tmp_rot * tmp_scl;
    cubemap->program_->SetMat4("transform", (float*)glm::value_ptr(t));

    glDepthFunc(GL_LEQUAL);

    unsigned int num_textures = (unsigned int)cubemap->textures_.size();
    if (num_textures != 0) {
      for (unsigned int i = 0; i < num_textures; i++) {
        if (cubemap->textures_.at(i)->loaded_ && cubemap->selected_texture_ == i) {
          char source[50] = "0";
          GLuint texture_id = cubemap->textures_.at(i)->texture_id_;
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture_id);
          cubemap->program_->SetSampler("background_texture", 0);
        }
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, cubemap->mesh_->virtual_buffer_object_);
    glBindVertexArray(cubemap->mesh_->virtual_array_object_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemap->mesh_->index_buffer_object_);
    glDrawElements(GL_TRIANGLES, (GLsizei)cubemap->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

    glDepthFunc(GL_LEQUAL);

  }

}

void RenderSystemDirectX11::render_elements_with_texture(ComponentManager* comp, Program* prog){


  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;

    //Search for the assigned transform of this renderer if it exists
    while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
      transform_iterator++;
    }

    if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
      t = &(transform_components->at(transform_iterator).data_);
    }

    //Only draw if the
    if (r->isInit_ && r->mesh_->isInit_) {
      glm::mat4 trans = glm::mat4(1.0f);
      if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }
      prog->SetMat4("transform", (float*)glm::value_ptr(trans));
      prog->SetBool("needs_light", r->needs_light_);

      // Activate textures if there are any
      for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
        if (r->textures_.at(j)->loaded_) {
          char source[50] = "0";
          GLuint textureId = r->textures_.at(j)->texture_id_;
          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, textureId);
          sprintf_s(source, "texture%d", (int)j + 1);
          prog->SetSampler(source, j);
        }
      }

      //Set the culling method
      if (r->mesh_->cull_type_ != last_cull) {
        switch (r->mesh_->cull_type_) {
        case 0:glCullFace(GL_FRONT); break;
        case 1:glCullFace(GL_BACK); break;
        case 2:glCullFace(GL_FRONT_AND_BACK); break;
        }

        last_cull = r->mesh_->cull_type_;
      }

      //c->draw_calls++;

      glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
      glBindVertexArray(r->mesh_->virtual_array_object_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
      glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);
      //glBindTexture(GL_TEXTURE_2D, 0);

    }
  }
}

void RenderSystemDirectX11::render_elements_depthmap(ComponentManager* comp, Program* prog){

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();

  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;

    //Search for the assigned transform of this renderer if it exists
    while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
      transform_iterator++;
    }

    if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
      t = &(transform_components->at(transform_iterator).data_);
    }

    //Only draw if the
    if (r->isInit_ && r->mesh_->isInit_) {
      glm::mat4 trans = glm::mat4(1.0f);
      if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }
      prog->SetMat4("transform", (float*)glm::value_ptr(trans));

      glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
      glBindVertexArray(r->mesh_->virtual_array_object_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
      glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

    }
  }
}

void RenderSystemDirectX11::render_light_elements(ComponentManager* comp, Program* prog, DirectionalLight* directional)
{
  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();


  //Update the program with the directional light values and the depthmap

  prog->SetMat4("directionalLightMatrix", glm::value_ptr(directional->lightMatrix_));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depthmap_directional_and_spotlight_shadows_->depthMapTexture_);
  prog->SetSampler("directional_depthmap", 0);

  prog->SetVec3("directional.position", directional->position_);
  prog->SetVec3("directional.direction", directional->direction_);
  prog->SetVec3("directional.diffuse", directional->diffuse_);
  prog->SetVec3("directional.specular", directional->specular_);
  prog->SetVec3("directional.ambient", directional->ambient_);

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;
    if (r != nullptr) {
      //Search for the assigned transform of this renderer if it exists
      while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
        transform_iterator++;
      }

      if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
        t = &(transform_components->at(transform_iterator).data_);
      }

      //Only draw if the renderer and mesh are init
      if (r->isInit_ && r->mesh_->isInit_) {
        glm::mat4 trans = glm::mat4(1.0f);
        if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }

        prog->SetMat4("transform", (float*)glm::value_ptr(trans));
        prog->SetBool("receivesShadows", r->receives_shadows_);
        prog->SetBool("needs_light", r->needs_light_);

        // Activate textures if there are any
        for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
          if (r->textures_.at(j)->loaded_) {
            char source[50] = "0";
            GLuint textureId = r->textures_.at(j)->texture_id_;
            glActiveTexture(GL_TEXTURE1 + j);
            glBindTexture(GL_TEXTURE_2D, textureId);
            sprintf_s(source, "texture%d", (int)j + 1);
            prog->SetSampler(source, j + 1);
          }
        }

        glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
        glBindVertexArray(r->mesh_->virtual_array_object_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
        glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

      }
    }
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystemDirectX11::render_light_elements(ComponentManager* comp, Program* prog, SpotLight* spotlight){
  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();


  //Update the program with the directional light values and the depthmap

  prog->SetMat4("spotLightMatrix", glm::value_ptr(spotlight->lightMatrix_));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depthmap_directional_and_spotlight_shadows_->depthMapTexture_);
  prog->SetSampler("spotlight_depthmap", 0);

  prog->SetVec3("spotlight.position", spotlight->position_);
  prog->SetVec3("spotlight.direction", spotlight->direction_);
  prog->SetVec3("spotlight.diffuse", spotlight->diffuse_);
  prog->SetVec3("spotlight.specular", spotlight->specular_);
  prog->SetFloat("spotlight.cutOff", spotlight->cut_off_);
  prog->SetFloat("spotlight.outerCutOff", spotlight->outer_cut_off_);
  prog->SetFloat("spotlight.constant", spotlight->constant_);
  prog->SetFloat("spotlight.linear", spotlight->linear_);
  prog->SetFloat("spotlight.quadratic", spotlight->quadratic_);

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;
    if (r != nullptr) {
      //Search for the assigned transform of this renderer if it exists
      while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
        transform_iterator++;
      }

      if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
        t = &(transform_components->at(transform_iterator).data_);
      }

      //Only draw if the renderer and mesh are init
      if (r->isInit_ && r->mesh_->isInit_) {
        glm::mat4 trans = glm::mat4(1.0f);
        if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }

        prog->SetMat4("transform", (float*)glm::value_ptr(trans));
        prog->SetBool("receivesShadows", r->receives_shadows_);
        prog->SetBool("needs_light", r->needs_light_);

        // Activate textures if there are any
        for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
          if (r->textures_.at(j)->loaded_) {
            char source[50] = "0";
            GLuint textureId = r->textures_.at(j)->texture_id_;
            glActiveTexture(GL_TEXTURE1 + j);
            glBindTexture(GL_TEXTURE_2D, textureId);
            sprintf_s(source, "texture%d", (int)j + 1);
            prog->SetSampler(source, j + 1);
          }
        }

        glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
        glBindVertexArray(r->mesh_->virtual_array_object_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
        glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

      }
    }
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystemDirectX11::render_light_elements(ComponentManager* comp, Program* prog, PointLight* pointlight){

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();


  //Update the program with the directional light values and the depthmap
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap_pointlight_shadows_->depthMapTexture_);
  prog->SetSampler("pointlight_depthmap", 0);

  prog->SetFloat("zfar", pointlight->zfar_);
  prog->SetVec3("lightPos", pointlight->position_);
  prog->SetMat4("lightSpaceMatrix[0]", glm::value_ptr(pointlight->lightMatrix_[0]));
  prog->SetMat4("lightSpaceMatrix[1]", glm::value_ptr(pointlight->lightMatrix_[1]));
  prog->SetMat4("lightSpaceMatrix[2]", glm::value_ptr(pointlight->lightMatrix_[2]));
  prog->SetMat4("lightSpaceMatrix[3]", glm::value_ptr(pointlight->lightMatrix_[3]));
  prog->SetMat4("lightSpaceMatrix[4]", glm::value_ptr(pointlight->lightMatrix_[4]));
  prog->SetMat4("lightSpaceMatrix[5]", glm::value_ptr(pointlight->lightMatrix_[5]));

  prog->SetVec3("pointlight.position", pointlight->position_);
  prog->SetVec3("pointlight.diffuse", pointlight->diffuse_);
  prog->SetVec3("pointlight.specular", pointlight->specular_);
  prog->SetFloat("pointlight.constant", pointlight->constant_);
  prog->SetFloat("pointlight.linear", pointlight->linear_);
  prog->SetFloat("pointlight.quadratic", pointlight->quadratic_);
  prog->SetFloat("pointlight.range", pointlight->range_);
  prog->SetFloat("point_far_plane", pointlight->zfar_);

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;
    if (r != nullptr) {
      //Search for the assigned transform of this renderer if it exists
      while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
        transform_iterator++;
      }

      if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
        t = &(transform_components->at(transform_iterator).data_);
      }

      //Only draw if the renderer and mesh are init
      if (r->isInit_ && r->mesh_->isInit_) {
        glm::mat4 trans = glm::mat4(1.0f);
        if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }

        prog->SetMat4("transform", (float*)glm::value_ptr(trans));
        prog->SetBool("receivesShadows", r->receives_shadows_);
        prog->SetBool("needs_light", r->needs_light_);

        // Activate textures if there are any
        for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
          if (r->textures_.at(j)->loaded_) {
            char source[50] = "0";
            GLuint textureId = r->textures_.at(j)->texture_id_;
            glActiveTexture(GL_TEXTURE1 + j);
            glBindTexture(GL_TEXTURE_2D, textureId);
            sprintf_s(source, "texture%d", (int)j + 1);
            prog->SetSampler(source, j + 1);
          }
        }

        glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
        glBindVertexArray(r->mesh_->virtual_array_object_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
        glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

      }
    }
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystemDirectX11::ForwardRendering(ComponentManager* comp){
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  render_scene_cubemap(comp);

  //Get camera values for the render element programs
  glm::mat4 cam_projection = glm::mat4(1.0f);
  glm::mat4 cam_view = glm::mat4(1.0f);
  glm::vec3 cam_position = glm::vec3(0.0f);

  CameraComponent* cam = comp->get_principal_camera();
  if (cam != nullptr) {
    cam_projection = cam->projection_;
    cam_view = cam->view_;
    cam_position = cam->position_;
  }
  else { cam_projection = glm::perspective(90.0f, (float)(window_.get()->width_ / window_.get()->height_), 0.01f, 2000.0f); }
 
  //Set camera values to all the programs

  render_elements_with_texture_->Use();
  render_elements_with_texture_->SetMat4("projection", glm::value_ptr(cam_projection));
  render_elements_with_texture_->SetMat4("view", glm::value_ptr(cam_view));
  render_elements_with_texture_->SetVec3("viewPos", cam_position);

  render_elements_directional_light_->Use();
  render_elements_directional_light_->SetMat4("projection", glm::value_ptr(cam_projection));
  render_elements_directional_light_->SetMat4("view", glm::value_ptr(cam_view));
  render_elements_directional_light_->SetVec3("viewPos", cam_position);

  render_elements_spotlight_->Use();
  render_elements_spotlight_->SetMat4("projection", glm::value_ptr(cam_projection));
  render_elements_spotlight_->SetMat4("view", glm::value_ptr(cam_view));
  render_elements_spotlight_->SetVec3("viewPos", cam_position);

  render_elements_pointlight_->Use();
  render_elements_pointlight_->SetMat4("projection", glm::value_ptr(cam_projection));
  render_elements_pointlight_->SetMat4("view", glm::value_ptr(cam_view));
  render_elements_pointlight_->SetVec3("viewPos", cam_position);

  bool first_pass = true;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //## DIRECTIONALS
  for (size_t i = 0; i < lights_.directional_.size(); ++i) {

    //Get the depthmap of the light and bind it's program and 
    DirectionalLight* dir = lights_.directional_.at(i).get();

    //Change the dir position to follow the camera

    if (dir->visible_) {

      dir->position_ = cam_position;
      dir->UpdateProjection();
      dir->UpdateView();

      //Render the shadow into the depthmap
      render_directional_and_spotlight_shadows_->Use();
      depthmap_directional_and_spotlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      render_directional_and_spotlight_shadows_->SetMat4("lightSpaceMatrix", glm::value_ptr(dir->lightMatrix_));
      render_elements_depthmap(comp, render_directional_and_spotlight_shadows_.get());
      depthmap_directional_and_spotlight_shadows_->UnsetBuffer();

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      render_elements_directional_light_->Use();
      //glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_BLEND);

      if (first_pass) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_ONE, GL_ZERO);
        render_light_elements(comp, render_elements_directional_light_.get(), dir);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        glBlendEquation(GL_FUNC_ADD);
        render_light_elements(comp, render_elements_directional_light_.get(), dir);
      }
    }
  }

  //## SPOTLIGHTS
  for (size_t i = 0; i < lights_.spot_.size(); ++i) {
    SpotLight* spot = lights_.spot_.at(i).get();

    if (spot->visible_) {

      spot->UpdateProjection();
      spot->UpdateView();

      //Render the shadow into the depthmap
      render_directional_and_spotlight_shadows_->Use();
      depthmap_directional_and_spotlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      render_directional_and_spotlight_shadows_->SetMat4("lightSpaceMatrix", glm::value_ptr(spot->lightMatrix_));
      render_elements_depthmap(comp, render_directional_and_spotlight_shadows_.get());
      depthmap_directional_and_spotlight_shadows_->UnsetBuffer();

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      render_elements_spotlight_->Use();
      //glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_BLEND);

      if (first_pass) {
        //glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_ONE, GL_ZERO);
        render_light_elements(comp, render_elements_spotlight_.get(), spot);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        glBlendEquation(GL_FUNC_ADD);
        render_light_elements(comp, render_elements_spotlight_.get(), spot);
      }
    }

  }

  //## POINTLIGHTS
  for (size_t i = 0; i < lights_.point_.size(); ++i) {
    PointLight* point = lights_.point_.at(i).get();

    if (point->visible_) {

      point->UpdateProjection();
      point->UpdateView();

      //Render the shadow into the depthmap
      render_pointlight_shadows_->Use();
      depthmap_pointlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      render_elements_depthmap(comp, render_pointlight_shadows_.get());
      depthmap_pointlight_shadows_->UnsetBuffer();

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      render_elements_pointlight_->Use();
      //glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_BLEND);

      if (first_pass) {
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_ONE, GL_ZERO);
        render_light_elements(comp, render_elements_pointlight_.get(), point);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        //glBlendEquation(GL_FUNC_ADD);
        render_light_elements(comp, render_elements_pointlight_.get(), point);
      }
    }

  }

  //If first pass still, render only the objects with the texture
  if (first_pass) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_ONE, GL_ZERO);
    render_elements_with_texture_->Use();
    render_elements_with_texture(comp, render_elements_with_texture_.get());
    glBlendFunc(GL_ONE, GL_ONE);
  }

  glDisable(GL_BLEND);
}

void RenderSystemDirectX11::render_deferred_light(Program* prog, DirectionalLight* directional){
  prog->SetMat4("directionalLightMatrix", glm::value_ptr(directional->lightMatrix_));
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, depthmap_directional_and_spotlight_shadows_->depthMapTexture_);
  prog->SetSampler("directional_depthmap", 3);

  prog->SetVec3("directional.position", directional->position_);
  prog->SetVec3("directional.direction", directional->direction_);
  prog->SetVec3("directional.ambient", directional->ambient_);
  prog->SetVec3("directional.diffuse", directional->diffuse_);
  prog->SetVec3("directional.specular", directional->specular_);

  deferred_framebuffer_->RenderDeferredToQuad();
}

void RenderSystemDirectX11::render_deferred_light(Program* prog, SpotLight* spotlight) {

  prog->SetMat4("spotLightMatrix", glm::value_ptr(spotlight->lightMatrix_));
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, depthmap_directional_and_spotlight_shadows_->depthMapTexture_);
  prog->SetSampler("spotlight_depthmap", 3);

  prog->SetVec3("spotlight.position", spotlight->position_);
  prog->SetVec3("spotlight.direction", spotlight->direction_);
  prog->SetVec3("spotlight.diffuse", spotlight->diffuse_);
  prog->SetVec3("spotlight.specular", spotlight->specular_);
  prog->SetFloat("spotlight.cutOff", spotlight->cut_off_);
  prog->SetFloat("spotlight.outerCutOff", spotlight->outer_cut_off_);
  prog->SetFloat("spotlight.constant", spotlight->constant_);
  prog->SetFloat("spotlight.linear", spotlight->linear_);
  prog->SetFloat("spotlight.quadratic", spotlight->quadratic_);

  deferred_framebuffer_->RenderDeferredToQuad();

}

void RenderSystemDirectX11::render_deferred_light(Program* prog, PointLight* pointlight) {

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap_pointlight_shadows_->depthMapTexture_);
  prog->SetSampler("pointlight_depthmap", 3);

  prog->SetVec3("pointlight.position", pointlight->position_);
  prog->SetVec3("pointlight.diffuse", pointlight->diffuse_);
  prog->SetVec3("pointlight.specular", pointlight->specular_);
  prog->SetFloat("pointlight.constant", pointlight->constant_);
  prog->SetFloat("pointlight.linear", pointlight->linear_);
  prog->SetFloat("pointlight.quadratic", pointlight->quadratic_);
  prog->SetFloat("pointlight.range", pointlight->range_);
  prog->SetFloat("point_far_plane", pointlight->zfar_);

  deferred_framebuffer_->RenderDeferredToQuad();

}

void RenderSystemDirectX11::render_deferred_elements_without_light(ComponentManager* comp, Program* prog){
  static size_t render_hash = typeid(RendererComponent).hash_code();

  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);

    //Only draw if the
    if (r->isInit_ && r->mesh_->isInit_) {
      glm::mat4 trans = glm::mat4(1.0f);
      prog->SetBool("needs_light", r->needs_light_);

    }
  }

  deferred_framebuffer_->RenderDeferredToQuad();

}

void RenderSystemDirectX11::DeferredRendering(ComponentManager* comp){

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  // Render scene's geometry/color data into gbuffer

  deferred_framebuffer_->SetBuffer();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  //Program* lights_program = component_manager_->deferred_rendering_lights_program_.get();

  CameraComponent* camera = comp->get_principal_camera();

  Program* dir_light_prog = deferred_rendering_directionallight_program_.get();
  dir_light_prog->Use();
  dir_light_prog->SetMat4("view", glm::value_ptr(camera->view_));
  dir_light_prog->SetVec3("viewPos", camera->position_);
  dir_light_prog->SetSampler("gPosition", 0);
  dir_light_prog->SetSampler("gNormal", 1);
  dir_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* spot_light_prog = deferred_rendering_spotlight_program_.get();
  spot_light_prog->Use();
  spot_light_prog->SetMat4("view", glm::value_ptr(camera->view_));
  spot_light_prog->SetVec3("viewPos", camera->position_);
  spot_light_prog->SetSampler("gPosition", 0);
  spot_light_prog->SetSampler("gNormal", 1);
  spot_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* point_light_prog = deferred_rendering_pointlight_program_.get();
  point_light_prog->Use();
  point_light_prog->SetMat4("view", glm::value_ptr(camera->view_));
  point_light_prog->SetVec3("viewPos", camera->position_);
  point_light_prog->SetSampler("gPosition", 0);
  point_light_prog->SetSampler("gNormal", 1);
  point_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* elements_without_light_prog = deferred_rendering_elements_with_texture_program_.get();
  elements_without_light_prog->Use();
  elements_without_light_prog->SetMat4("view", glm::value_ptr(camera->view_));
  elements_without_light_prog->SetVec3("viewPos", camera->position_);
  elements_without_light_prog->SetSampler("gPosition", 0);
  elements_without_light_prog->SetSampler("gNormal", 1);
  elements_without_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* elements_program = deferred_rendering_geometry_program_.get();
  elements_program->Use();
  elements_program->SetMat4("projection", glm::value_ptr(camera->projection_));
  elements_program->SetMat4("view", glm::value_ptr(camera->view_));

  //Render all elements to split the position, normal and albedo
  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();

  unsigned char last_cull = -1;
  size_t transform_iterator = 0;
  for (size_t it = 0; it < render_size; it++) {

    size_t id = renderer_components->at(it).entity_id_;
    RendererComponent* r = &(renderer_components->at(it).data_);
    TransformComponent* t = nullptr;

    //Search for the assigned transform of this renderer if it exists
    while (transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ < id) {
      transform_iterator++;
    }

    if (transf_size != 0 && transform_iterator < transf_size && transform_components->at(transform_iterator).entity_id_ == id) {
      t = &(transform_components->at(transform_iterator).data_);
    }

    //Only draw if the
    if (r->isInit_ && r->mesh_->isInit_) {
      glm::mat4 trans = glm::mat4(1.0f);
      if (nullptr != t) { trans = comp->get_parent_transform_matrix(id); }
      elements_program->SetMat4("transform", glm::value_ptr(trans));

      // Activate textures if there are any
      for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
        if (r->textures_.at(j)->loaded_) {
          GLuint textureId = r->textures_.at(j)->texture_id_;
          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, textureId);
          elements_program->SetSampler("texture_diffuse", j);
        }
      }

      //Set the culling method
      if (r->mesh_->cull_type_ != last_cull) {
        switch (r->mesh_->cull_type_) {
        case 0:glCullFace(GL_FRONT); break;
        case 1:glCullFace(GL_BACK); break;
        case 2:glCullFace(GL_FRONT_AND_BACK); break;
        }

        last_cull = r->mesh_->cull_type_;
      }

      glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
      glBindVertexArray(r->mesh_->virtual_array_object_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
      glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

    }
  }

  //Unset the deferred framebuffer to render into the default one
  deferred_framebuffer_->UnsetBuffer();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_scene_cubemap(comp);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, deferred_framebuffer_->position_texture_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, deferred_framebuffer_->normal_texture_);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, deferred_framebuffer_->albedospec_texture_);

  bool first_pass = true;

  //Render each light with the quad of the scene
  //Directional lights
  dir_light_prog->Use();
  for (size_t i = 0; i < lights_.directional_.size(); ++i) {
    DirectionalLight* dir = lights_.directional_.at(i).get();

    if (dir->visible_) {
      //Render the shadow into the depthmap
      render_directional_and_spotlight_shadows_->Use();
      depthmap_directional_and_spotlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glCullFace(GL_BACK);
      render_directional_and_spotlight_shadows_->SetMat4("lightSpaceMatrix", glm::value_ptr(dir->lightMatrix_));
      render_elements_depthmap(comp, render_directional_and_spotlight_shadows_.get());
      depthmap_directional_and_spotlight_shadows_->UnsetBuffer();
      glCullFace(GL_FRONT);

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      //Render the scene quad with the shadow information
      deferred_rendering_directionallight_program_.get()->Use();
      glEnable(GL_BLEND);
      if (first_pass) {
        glBlendFunc(GL_ONE, GL_ZERO);
        render_deferred_light(deferred_rendering_directionallight_program_.get(), dir);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        render_deferred_light(deferred_rendering_directionallight_program_.get(), dir);
      }

    }
  }

  //Spot lights
  spot_light_prog->Use();
  for (size_t i = 0; i < lights_.spot_.size(); ++i) {
    SpotLight* spot = lights_.spot_.at(i).get();

    if (spot->visible_) {

      spot->UpdateProjection();
      spot->UpdateView();

      //Render the shadow into the depthmap
      render_directional_and_spotlight_shadows_->Use();
      depthmap_directional_and_spotlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      render_directional_and_spotlight_shadows_->SetMat4("lightSpaceMatrix", glm::value_ptr(spot->lightMatrix_));
      render_elements_depthmap(comp, render_directional_and_spotlight_shadows_.get());
      depthmap_directional_and_spotlight_shadows_->UnsetBuffer();
      glCullFace(GL_FRONT);

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      //Render the scene quad with the shadow information
      deferred_rendering_spotlight_program_->Use();
      glEnable(GL_BLEND);
      if (first_pass) {
        glBlendFunc(GL_ONE, GL_ZERO);
        render_deferred_light(deferred_rendering_spotlight_program_.get(), spot);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        render_deferred_light(deferred_rendering_spotlight_program_.get(), spot);
      }

    }
  }

  //Point lights
  point_light_prog->Use();
  for (size_t i = 0; i < lights_.point_.size(); ++i) {
    PointLight* point = lights_.point_.at(i).get();

    if (point->visible_) {

      point->UpdateProjection();
      point->UpdateView();

      //Render the shadow into the depthmap
      render_pointlight_shadows_->Use();
      depthmap_pointlight_shadows_->SetBuffer();
      glDisable(GL_BLEND);
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      render_pointlight_shadows_->SetFloat("zfar", point->zfar_);
      render_pointlight_shadows_->SetVec3("lightPos", point->position_);
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[0]", glm::value_ptr(point->lightMatrix_[0]));
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[1]", glm::value_ptr(point->lightMatrix_[1]));
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[2]", glm::value_ptr(point->lightMatrix_[2]));
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[3]", glm::value_ptr(point->lightMatrix_[3]));
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[4]", glm::value_ptr(point->lightMatrix_[4]));
      render_pointlight_shadows_->SetMat4("lightSpaceMatrix[5]", glm::value_ptr(point->lightMatrix_[5]));
      render_elements_depthmap(comp, render_pointlight_shadows_.get());
      depthmap_pointlight_shadows_->UnsetBuffer();
      glCullFace(GL_FRONT);

      //Reset viewport and render the elements with that depthmap and the directional light
      glViewport(0, 0, window_.get()->width_, window_.get()->height_);

      //Render the scene quad with the shadow information
      deferred_rendering_pointlight_program_->Use();
      glEnable(GL_BLEND);
      if (first_pass) {
        glBlendFunc(GL_ONE, GL_ZERO);
        render_deferred_light(deferred_rendering_pointlight_program_.get(), point);
        first_pass = false;
        glBlendFunc(GL_ONE, GL_ONE);
      }
      else {
        render_deferred_light(deferred_rendering_pointlight_program_.get(), point);
      }

    }
  }

  //If first pass still, render only the objects with the texture
  if (first_pass) {
    deferred_rendering_elements_with_texture_program_->Use();
    render_deferred_elements_without_light(comp, deferred_rendering_elements_with_texture_program_.get());
  }

  glDisable(GL_BLEND);
}
/**/
