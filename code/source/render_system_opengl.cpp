#include <render_system_opengl.hpp>


#ifdef RENDER_OPENGL
#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#endif

#ifdef RENDER_OPENGL
RenderSystemOpenGL::RenderSystemOpenGL(int window_w, int window_h){
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  //## INIT THE RENDERING WINDOW
  window_ = Window::create_opengl(window_w, window_h, "Ventana", { 0.5f, 0.5f, 0.5f, 1.0f });
  if (window_ == nullptr) {
    printf("Couldn't init window");
    exit(EXIT_FAILURE);
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // Setup Platform/RendererComponent bindings
  ImGui_ImplGlfw_InitForOpenGL(window_.get()->window, true);
  ImGui_ImplOpenGL3_Init();
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  //Initialize GLEW here, after the context is created
  //Start glfw or exit if failed
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    exit(EXIT_FAILURE);
  }

  //## FORWARD RENDERING

  render_elements_with_texture_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_ELEMENTS_WITH_TEXTURE_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_ELEMENTS_WITH_TEXTURE_FRAGMENT_SHADER) };
    render_elements_with_texture_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_elements_with_texture_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_elements_with_texture_->Link();
  }


  render_elements_directional_light_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_DIRECTIONAL_ELEMENTS_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_DIRECTIONAL_ELEMENTS_FRAGMENT_SHADER) };
    render_elements_directional_light_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_elements_directional_light_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_elements_directional_light_->Link();
  }

  render_elements_spotlight_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_SPOTLIGHT_ELEMENTS_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_SPOTLIGHT_ELEMENTS_FRAGMENT_SHADER) };
    render_elements_spotlight_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_elements_spotlight_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_elements_spotlight_->Link();
  }


  render_elements_pointlight_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_POINTLIGHT_ELEMENTS_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_POINTLIGHT_ELEMENTS_FRAGMENT_SHADER) };
    render_elements_pointlight_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_elements_pointlight_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_elements_pointlight_->Link();
  }

  //## DEFERRED RENDERING

  deferred_rendering_geometry_program_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(DEFERRED_GEOMETRY_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(DEFERRED_GEOMETRY_FRAGMENT_SHADER) };
    deferred_rendering_geometry_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    deferred_rendering_geometry_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    deferred_rendering_geometry_program_->Link();
  }

  deferred_rendering_directionallight_program_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(DEFERRED_DIRECTIONAL_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(DEFERRED_DIRECTIONAL_FRAGMENT_SHADER) };
    deferred_rendering_directionallight_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    deferred_rendering_directionallight_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    deferred_rendering_directionallight_program_->Link();
  }

  deferred_rendering_spotlight_program_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(DEFERRED_SPOTLIGHT_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(DEFERRED_SPOTLIGHT_FRAGMENT_SHADER) };
    deferred_rendering_spotlight_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    deferred_rendering_spotlight_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    deferred_rendering_spotlight_program_->Link();
  }

  deferred_rendering_pointlight_program_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(DEFERRED_POINTLIGHT_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(DEFERRED_POINTLIGHT_FRAGMENT_SHADER) };
    deferred_rendering_pointlight_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    deferred_rendering_pointlight_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    deferred_rendering_pointlight_program_->Link();
  }

  deferred_rendering_elements_with_texture_program_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(DEFERRED_ELEMENTS_WITH_TEXTURE_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(DEFERRED_ELEMENTS_WITH_TEXTURE_FRAGMENT_SHADER) };
    deferred_rendering_elements_with_texture_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    deferred_rendering_elements_with_texture_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    deferred_rendering_elements_with_texture_program_->Link();
  }

  //## SHARED PROGRAMS FOR THE DIRECTIONAL, SPOT AND POINT SHADOWS
  render_directional_and_spotlight_shadows_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_DIRECTIONAL_AND_SPOTLIGHT_SHADOWS_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_DIRECTIONAL_AND_SPOTLIGHT_SHADOWS_FRAGMENT_SHADER) };
    render_directional_and_spotlight_shadows_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_directional_and_spotlight_shadows_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_directional_and_spotlight_shadows_->Link();
  }

  render_pointlight_shadows_ = std::make_unique<Program>();
  {
    std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(RENDER_POINTLIGHT_SHADOWS_VERTEX_SHADER) };
    std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(RENDER_POINTLIGHT_SHADOWS_FRAGMENT_SHADER) };
    std::unique_ptr<char> src_g{ Shader::ReadShaderFromFile(RENDER_POINTLIGHT_SHADOWS_GEOMETRY_SHADER) };
    render_pointlight_shadows_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
    render_pointlight_shadows_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
    render_pointlight_shadows_->AttachShader(std::make_unique<Shader>(src_g.get(), ShaderType::GEOMETRY).get());
    render_pointlight_shadows_->Link();
  }

  depthmap_directional_and_spotlight_shadows_ = std::make_unique<DepthMap>(SHADOWS_DIMENSIONS, DepthMapType::kTypeQuad);
  depthmap_pointlight_shadows_ = std::make_unique<DepthMap>(SHADOWS_DIMENSIONS, DepthMapType::kTypeCubeMap);

  deferred_framebuffer_ = std::make_unique<DeferredFramebuffer>(window_->GetWindowWidth(), window_->GetWindowHeight());

  IsReady_ = true;

  resource_list_.InitResources();


  //Check if text started correctly
  free_type_init = InitTextResources();
}

RenderSystemOpenGL::~RenderSystemOpenGL(){
    if (free_type_init) {
        std::map<char, Character>::iterator it;
        for (it = characters_.begin(); it != characters_.end(); it++){
            glDeleteTextures(1, &(it->second.TextureID));
        }

        glDeleteBuffers(1, &text_render_VAO);
        glDeleteBuffers(1, &text_render_VBO);
    }
}

void RenderSystemOpenGL::Render(ComponentManager* comp){
  //*/
  DeferredRendering(comp);
  /*/
  ForwardRendering(comp);
  /**/

  window_->render();
}

void RenderSystemOpenGL::Update(){
  //Update
  window_->clear();
  window_->update_delta();
  window_->detect_events();

  //If the deferred frame buffer size is different from the window size, resize it
  int w = window_->GetWindowWidth(), h = window_->GetWindowHeight();
  if (w != deferred_framebuffer_->dimensions_x ||
      h != deferred_framebuffer_->dimensions_y) {
      ResizeBuffers(w, h);
  }
}

Window* RenderSystemOpenGL::getWindow()
{
  return window_.get();
}

void RenderSystemOpenGL::ResetResources(){
    resource_list_.meshes_.clear();
    resource_list_.textures_.clear();

    for (unsigned int i = 0; i < resource_list_.screen_texts_.size();++i) {
        resource_list_.screen_texts_.at(i).reset();
    }
    resource_list_.screen_texts_.resize(0);

    for (unsigned int i = 0; i < lights_.directional_.size(); ++i) {
        lights_.directional_.at(i).reset();
    }
    lights_.directional_.resize(0);


    for (unsigned int i = 0; i < lights_.spot_.size(); ++i) {
        lights_.spot_.at(i).reset();
    }
    lights_.spot_.resize(0);


    for (unsigned int i = 0; i < lights_.point_.size(); ++i) {
        lights_.point_.at(i).reset();
    }
    lights_.point_.resize(0);
}

void RenderSystemOpenGL::ResizeBuffers(int w, int h){
    deferred_framebuffer_->ResizeBuffer(w, h);

    //ResetTextResources();
}

bool RenderSystemOpenGL::InitTextResources(){

    InitLetterCharacters();

    //Init buffers for rendering
    glGenVertexArrays(1, &text_render_VAO);
    glGenBuffers(1, &text_render_VBO);
    glBindVertexArray(text_render_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, text_render_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    GLuint indexes_[] = {
        0, 1, 3,
        1, 2, 3
    };
    glGenBuffers(1, &text_render_IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_render_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes_), indexes_, GL_STATIC_DRAW);

    //Init the program for rendering text
    render_text_program_ = std::make_unique<Program>();
    {
        std::unique_ptr<char> src_v{ Shader::ReadShaderFromFile(TEXT_RENDERING_VERTEX_SHADER) };
        std::unique_ptr<char> src_f{ Shader::ReadShaderFromFile(TEXT_RENDERING_PIXEL_SHADER) };
        render_text_program_->AttachShader(std::make_unique<Shader>(src_v.get(), ShaderType::VERTEX).get());
        render_text_program_->AttachShader(std::make_unique<Shader>(src_f.get(), ShaderType::FRAGMENT).get());
        render_text_program_->Link();
    }

    //Projection for the text
    text_proj = glm::ortho(0.0f, (float)window_->GetWindowWidth(), 0.0f, (float)window_->GetWindowHeight());

    return true;
}

bool RenderSystemOpenGL::InitLetterCharacters(){

    //Init the FreeType Library
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, "../data/fonts/arial.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return false;
    }


    FT_Set_Pixel_Sizes(face, 0, 48);

    //Get previous alignment to restore it after the initialization
    int alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Init each letter
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            printf("Error loading char");
            return false;
        }
        else {
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            characters_.insert(std::pair<char, Character>(c, character));

        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    //Clean resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void RenderSystemOpenGL::ResetTextResources(){

    //Clean the textures
    for (int i = 0; i < characters_.size(); ++i) {
        glDeleteTextures(1, &characters_[i].TextureID);
    }

    characters_.clear();

    InitLetterCharacters();


    //Projection for the text
    text_proj = glm::ortho(0.0f, (float)window_->GetWindowWidth(), 0.0f, (float)window_->GetWindowHeight());
}

void RenderSystemOpenGL::RenderText(std::string text, float screen_x, float screen_y, float scale, glm::vec3 color){
    // activate corresponding render state	
    render_text_program_->Use();
    render_text_program_->SetVec3("textColor", color);
    render_text_program_->SetMat4("projection", (float*)glm::value_ptr(text_proj));
    glActiveTexture(GL_TEXTURE0);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++){
        Character ch = characters_[*c];

        float xpos = screen_x + ch.Bearing.x * scale;
        float ypos = screen_y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[4][4] = {
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindVertexArray(text_render_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, text_render_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        // be sure to use glBufferSubData and not glBufferData

        // render quad

        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_render_IBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        screen_x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystemOpenGL::render_scene_cubemap(ComponentManager* comp){
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
      cam_projection = cam->get_projection();
      cam_view = cam->get_view();
      cam_position = cam->get_position();
    }
    else {
        cam_projection = glm::perspective(90.0f,
            (float)(window_.get()->GetWindowWidth() / window_.get()->GetWindowHeight()),
            0.01f, 2000.0f);
    }

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

void RenderSystemOpenGL::render_elements_with_texture(ComponentManager* comp, Program* prog){


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
      if (nullptr != t) { trans = t->GetTransform(); }
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

void RenderSystemOpenGL::render_elements_depthmap(ComponentManager* comp, Program* prog){

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
      if (nullptr != t) { trans = t->GetTransform(); }
      prog->SetMat4("transform", (float*)glm::value_ptr(trans));

      glBindBuffer(GL_ARRAY_BUFFER, r->mesh_->virtual_buffer_object_);
      glBindVertexArray(r->mesh_->virtual_array_object_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh_->index_buffer_object_);
      glDrawElements(GL_TRIANGLES, (GLsizei)r->mesh_->indexes_.size(), GL_UNSIGNED_INT, nullptr);

    }
  }
}

void RenderSystemOpenGL::render_light_elements(ComponentManager* comp, Program* prog, DirectionalLight* directional)
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
        if (nullptr != t) { trans = t->GetTransform(); }

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

void RenderSystemOpenGL::render_light_elements(ComponentManager* comp, Program* prog, SpotLight* spotlight){
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
        if (nullptr != t) { trans = t->GetTransform(); }

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

void RenderSystemOpenGL::render_light_elements(ComponentManager* comp, Program* prog, PointLight* pointlight){

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
        if (nullptr != t) { trans = t->GetTransform(); }

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

void RenderSystemOpenGL::ForwardRendering(ComponentManager* comp) {

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  render_scene_cubemap(comp);

  //Get camera values for the render element programs
  glm::mat4 cam_projection = glm::mat4(1.0f);
  glm::mat4 cam_view = glm::mat4(1.0f);
  glm::vec3 cam_position = glm::vec3(0.0f);

  CameraComponent* cam = comp->get_principal_camera();
  if (cam != nullptr) {
    cam_projection = cam->get_projection();
    cam_view = cam->get_view();
    cam_position = cam->get_position();
  }
  else { 
      cam_projection = glm::perspective(90.0f, 
                            (float)(window_.get()->GetWindowWidth() / window_.get()->GetWindowHeight()), 
                            0.01f, 2000.0f); 
  }
 
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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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

  //Render text
  glCullFace(GL_FRONT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (unsigned int i = 0; i < resource_list_.screen_texts_.size(); i++) {
      RenderingText* r = resource_list_.screen_texts_.at(i).get();
      RenderText(r->text_, r->pos_x, r->pos_y, r->scale_, r->color_);
  }
  glDisable(GL_BLEND);
}

void RenderSystemOpenGL::render_deferred_light(Program* prog, DirectionalLight* directional){
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

void RenderSystemOpenGL::render_deferred_light(Program* prog, SpotLight* spotlight) {

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

void RenderSystemOpenGL::render_deferred_light(Program* prog, PointLight* pointlight) {

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

void RenderSystemOpenGL::render_deferred_elements_without_light(ComponentManager* comp, Program* prog){
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

static float offset = 0.0f;
void RenderSystemOpenGL::DeferredRendering(ComponentManager* comp){

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  // Render scene's geometry/color data into gbuffer
  deferred_framebuffer_->SetBuffer();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);

  CameraComponent* camera = comp->get_principal_camera();
  
  glm::vec3 cam_pos = glm::vec3(0.0f);
  glm::mat4 cam_proj = glm::mat4(1.0f);
  glm::mat4 cam_view = glm::mat4(1.0f);

  if (camera != nullptr) {
      cam_pos = camera->get_position();
      cam_proj = camera->get_projection();
      cam_view = camera->get_view();
  }

  Program* dir_light_prog = deferred_rendering_directionallight_program_.get();
  dir_light_prog->Use();
  dir_light_prog->SetMat4("view", glm::value_ptr(cam_view));
  dir_light_prog->SetVec3("viewPos", cam_pos);
  dir_light_prog->SetSampler("gPosition", 0);
  dir_light_prog->SetSampler("gNormal", 1);
  dir_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* spot_light_prog = deferred_rendering_spotlight_program_.get();
  spot_light_prog->Use();
  spot_light_prog->SetMat4("view", glm::value_ptr(cam_view));
  spot_light_prog->SetVec3("viewPos", cam_pos);
  spot_light_prog->SetSampler("gPosition", 0);
  spot_light_prog->SetSampler("gNormal", 1);
  spot_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* point_light_prog = deferred_rendering_pointlight_program_.get();
  point_light_prog->Use();
  point_light_prog->SetMat4("view", glm::value_ptr(cam_view));
  point_light_prog->SetVec3("viewPos", cam_pos);
  point_light_prog->SetSampler("gPosition", 0);
  point_light_prog->SetSampler("gNormal", 1);
  point_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* elements_without_light_prog = deferred_rendering_elements_with_texture_program_.get();
  elements_without_light_prog->Use();
  elements_without_light_prog->SetMat4("view", glm::value_ptr(cam_view));
  elements_without_light_prog->SetVec3("viewPos", cam_pos);
  elements_without_light_prog->SetSampler("gPosition", 0);
  elements_without_light_prog->SetSampler("gNormal", 1);
  elements_without_light_prog->SetSampler("gAlbedoSpec", 2);

  Program* elements_program = deferred_rendering_geometry_program_.get();
  elements_program->Use();
  elements_program->SetMat4("projection", glm::value_ptr(cam_proj));
  elements_program->SetMat4("view", glm::value_ptr(cam_view));


  //Render all elements to split the position, normal and albedo
  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transf_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  size_t transf_size = transform_components->size();

  unsigned char last_cull = -1;
  unsigned int last_texture = -1;
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
      if (nullptr != t) { trans = t->GetTransform(); }
      elements_program->SetMat4("transform", glm::value_ptr(trans));

      // Activate textures if there are any
      for (unsigned int j = 0; j < (unsigned int)r->textures_.size(); j++) {
        if (r->textures_.at(j)->loaded_) {
          GLuint textureId = r->textures_.at(j)->texture_id_;
          glActiveTexture(GL_TEXTURE0 + j);

          if (last_texture != textureId) {
              last_texture = textureId;
              glBindTexture(GL_TEXTURE_2D, last_texture);
          }

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

      if (last_loaded_mesh != r->mesh_) {
          last_loaded_mesh = r->mesh_;

          glBindBuffer(GL_ARRAY_BUFFER, last_loaded_mesh->virtual_buffer_object_);
          glBindVertexArray(last_loaded_mesh->virtual_array_object_);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_loaded_mesh->index_buffer_object_);
      }

      
      glDrawElements(GL_TRIANGLES, (GLsizei)last_loaded_mesh->indexes_.size(), GL_UNSIGNED_INT, nullptr);

    }
  }

  //Unset the deferred framebuffer to render into the default one
  deferred_framebuffer_->UnsetBuffer();
  //postprocess_framebuffer_->SetBuffer();
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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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
      glViewport(0, 0, window_.get()->GetWindowWidth(), window_.get()->GetWindowHeight());

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

  //postprocess_framebuffer_->UnsetBuffer();

  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, postprocess_framebuffer_->texture_id_);
  //postprocess_program_->Use();
  //postprocess_program_->SetSampler("texture1", 0);
  //offset += 1.0f;
  //postprocess_program_->SetFloat("offset", offset);

  //glBindBuffer(GL_ARRAY_BUFFER, postprocess_vbo_);
  //glBindVertexArray(postprocess_vao_);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postprocess_ibo_);
  //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  
  // TODO: Set postprocess texture to shader and aply postprocess

  //glDisable(GL_CULL_FACE);

  //Only draw text if it's ready
  if(free_type_init){

      glCullFace(GL_FRONT);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for (unsigned int i = 0; i < resource_list_.screen_texts_.size(); i++) {
          RenderingText* r = resource_list_.screen_texts_.at(i).get();
          RenderText(r->text_, r->pos_x, r->pos_y, r->scale_, r->color_);
      }
  }
  glDisable(GL_BLEND);


}
#endif