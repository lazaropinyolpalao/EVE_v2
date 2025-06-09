#include <resources.hpp>


// #### RESOURCES ####

#ifdef RENDER_OPENGL
std::shared_ptr<TinyObj> Resources::addMesh(std::string filepath) {

  if (filepath.empty()) { return std::shared_ptr<TinyObj>(nullptr); }

  std::shared_ptr<TinyObj> mesh = std::make_shared<TinyObj>();

  mesh->LoadObj(filepath);
  mesh->InitBuffer();

  meshes_.push_back(std::move(mesh));

  return meshes_.back();
}

std::shared_ptr<Texture> Resources::addTexture(std::string filepath) {

  if (filepath.empty()) { return std::shared_ptr<Texture>(nullptr); }

  std::shared_ptr<Texture> text = std::make_shared<Texture>();

  text->LoadTexture(filepath);

  textures_.push_back(std::move(text));

  return textures_.back();
}


bool Resources::InitResources() {
  cubemap_ = std::make_unique<Cubemap>();

  std::string skybox = "../data/cubemap/textures/skybox.jpg";
  //std::string space = "../data/cubemap/textures/space.jpg";
  //std::string field = "../data/cubemap/textures/field.jpg";
  cubemap_->addTexture(skybox);
  //cubemap_->addTexture(space);
  //cubemap_->addTexture(field);
  


  return true;
}
#endif
#ifdef RENDER_DIRECTX11
std::shared_ptr<TinyObj> Resources::addMesh(ID3D11Device* dev, ID3D11DeviceContext* devCon, std::string filepath) {

    if (filepath.empty()) { return nullptr; }

  meshes_.emplace_back(new TinyObj())->LoadObj(filepath);
  meshes_.back()->InitBuffer(dev, devCon);

  return meshes_.back();
}

std::shared_ptr<Texture> Resources::addTexture(ID3D11Device* dev, std::string filepath) {

  if (filepath.empty()) { return std::shared_ptr<Texture>(nullptr); }

  std::shared_ptr<Texture> text = std::make_shared<Texture>();

  text->LoadTexture(dev, filepath);

  textures_.push_back(std::move(text));

  return textures_.back();
}


bool Resources::InitResources(ID3D11Device* dev) {
  cubemap_ = std::make_unique<Cubemap>(dev);

  std::string skybox = "../data/cubemap/textures/skybox.jpg";
  std::string space = "../data/cubemap/textures/space.jpg";
  std::string field = "../data/cubemap/textures/field.jpg";
  cubemap_->addTexture(dev, skybox);
  cubemap_->addTexture(dev, space);
  cubemap_->addTexture(dev, field);

  return true;
}
#endif
void Resources::toggleCubeMap() {
  //cubemap_.get()->visible_ = !cubemap_.get()->visible_;
  cubemap_->visible_ = !cubemap_->visible_;
}

void Resources::ClearResources(){
  //-Textures
  for (unsigned int i = 0; i < textures_.size(); ++i) {
    textures_[i].reset();
  }
  textures_.clear();

  //-Meshes
  for (unsigned int i = 0; i < meshes_.size(); ++i) {
    meshes_[i].reset();
  }
  meshes_.clear();
}

RenderingText* Resources::addTextToRender(std::string text, float x, float y, glm::vec3 color, float s){

    if (text.size() > TEXT_RENDERING_MAX_SIZE) {
        text.resize(TEXT_RENDERING_MAX_SIZE, '\0');
    }


    std::unique_ptr<RenderingText> t = std::make_unique<RenderingText>(text, x, y, color, s);
    screen_texts_.push_back(std::move(t));

    return screen_texts_.back().get();
}

RenderingText* Resources::addTextToRender(){
    std::unique_ptr<RenderingText> t = std::make_unique<RenderingText>();
    screen_texts_.push_back(std::move(t));

    return screen_texts_.back().get();
}

Audio* Resources::addAudioFile(std::string filepath) {

  if (filepath.empty()) { return nullptr; }

  std::unique_ptr<Audio> audio = std::make_unique<Audio>();

  audio->loadWAVFile(filepath);

  audios_.push_back(std::move(audio));

  return audios_.back().get();

}


std::shared_ptr<TinyObj> Resources::getMeshByName(std::string objfile){

    std::shared_ptr<TinyObj> found = nullptr;

    for (unsigned int i = 0; found == nullptr && i < meshes_.size(); ++i) {
        if (objfile.compare(meshes_.at(i)->name_) == 0) {
            found = meshes_.at(i);
        }
    }

    return found;
}

std::shared_ptr<Texture> Resources::getTextureByName(std::string texturefile) {
    std::shared_ptr<Texture> found = nullptr;

    for (unsigned int i = 0; found == nullptr && i < textures_.size(); ++i) {
        if (texturefile.compare(textures_.at(i)->name_) == 0) {
            found = textures_.at(i);
        }
    }
    return found;
}
