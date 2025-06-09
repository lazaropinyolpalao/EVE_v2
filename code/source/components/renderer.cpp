#include <renderer.hpp>


// #### RENDERER COMPONENT ####
RendererComponent::RendererComponent() {
    mesh_ = nullptr;
    isInit_ = false;

    needs_light_ = true;
    casts_shadows_ = true;
    receives_shadows_ = true;
}

//MULTITHREAD UNSAFE
//Variant that makes a shared_ptr of the mesh to avoid storing the vertices twice
RendererComponent* RendererComponent::Init(std::shared_ptr<TinyObj> mesh) {
    if (this == nullptr) { return nullptr; }
    if (mesh.get() != nullptr) {
        mesh_ = mesh;
        isInit_ = true;
    }

    return this;
}

RendererComponent* RendererComponent::AddTexture(std::shared_ptr<Texture> texture) {
    if (texture != nullptr) {
        textures_.push_back(texture);
    }

    return this;
}

RendererComponent* RendererComponent::ChangeMesh(std::shared_ptr<TinyObj> new_mesh) {
    if (new_mesh != nullptr && new_mesh != mesh_) {
        mesh_ = new_mesh;

        if (!new_mesh.get()->isInit_) { isInit_ = false; }
        else { isInit_ = true; }
    }

    return this;
}
