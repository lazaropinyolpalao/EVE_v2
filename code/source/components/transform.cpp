#include <transform.hpp>

// #### TRANSFORM COMPONENT ####

TransformComponent::TransformComponent() {
    scale_ = glm::vec3(1.0f);
    rotation_ = glm::vec3(0.0f);
    position_ = glm::vec3(0.0f);
    parent = glm::mat4(1.0f);    
    relative = glm::mat4(1.0f);
    absolute = glm::mat4(1.0f);
    updated_ = false;
}

glm::mat4 TransformComponent::GetRelativeMatrix() {
    return relative;
}

glm::mat4 TransformComponent::GetParentMatrix(){
    return parent;
}

glm::mat4 TransformComponent::GetTransform(){
    return absolute;
}

glm::vec3 TransformComponent::GetScale(){
    return scale_;
}

glm::vec3 TransformComponent::GetRotation(){
    return rotation_;
}

glm::vec3 TransformComponent::GetPosition(){
    return position_;
}

TransformComponent* TransformComponent::SetScale(float x, float y, float z) {
    if (this == nullptr) { return nullptr; }

    scale_ = glm::vec3(x, y, z);

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::SetScale(glm::vec3 scl) {
    if (this == nullptr) { return nullptr; }

    scale_ = scl;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::AddRotationX(float x, bool radians) {
    if (this == nullptr) { return nullptr; }

    if (radians) { x = glm::degrees(x); }
    rotation_.x += x;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::AddRotationY(float y, bool radians) {
    if (this == nullptr) { return nullptr; }

    if (radians) { y = glm::degrees(y); }
    rotation_.y += y;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::AddRotationZ(float z, bool radians) {
    if (this == nullptr) { return nullptr; }

    if (radians) { z = glm::degrees(z); }
    rotation_.z += z;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::AddRotation(float x, float y, float z, bool radians) {
    if (this == nullptr) { return nullptr; }

    if (radians) {
        x = glm::degrees(x);
        y = glm::degrees(y);
        z = glm::degrees(z);
    }
    rotation_.x += x;
    rotation_.y += y;
    rotation_.z += z;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::AddRotation(glm::vec3 rot, bool radians) {
    if (this == nullptr) { return nullptr; }

    if (radians) {
        rot.x = glm::degrees(rot.x);
        rot.y = glm::degrees(rot.y);
        rot.z = glm::degrees(rot.z);
    }
    rotation_.x += rot.x;
    rotation_.y += rot.y;
    rotation_.z += rot.z;


    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::SetTranslation(float x, float y, float z) {
    if (this == nullptr) { return nullptr; }
    position_ = glm::vec3(x, y, z);

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::SetTranslation(glm::vec3 tsl) {
    if (this == nullptr) { return nullptr; }
    position_ = tsl;

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::SetRotation(float x, float y, float z, bool radians){
    if (this == nullptr) { return nullptr; }

    if (radians) {
        rotation_ = glm::vec3(glm::degrees(x), glm::degrees(y), glm::degrees(z));
    }
    else {
        rotation_ = glm::vec3(x, y, z);
    }

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

TransformComponent* TransformComponent::SetRotation(glm::vec3 rotation, bool radians){
    if (this == nullptr) { return nullptr; }

    if (radians) {
        rotation_ = glm::vec3(glm::degrees(rotation.x), glm::degrees(rotation.y), glm::degrees(rotation.z));
    }
    else {
        rotation_ = glm::vec3(rotation.x, rotation.y, rotation.z);
    }

    UpdateRelativeMatrix();
    UpdateTransform();

    return this;
}

void TransformComponent::SetParentMatrix(glm::mat4 parent_matrix){
    parent = parent_matrix;

    UpdateTransform();
}

void TransformComponent::UpdateTransform(){
    //absolute = glm::inverse(parent) * relative;

    //Decompose the parent transformation in order to do TRS

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(parent, scale, rotation, translation, skew, perspective);

    //absolute = relative * glm::inverse(parent);

    //Temporal rotation
    glm::quat pitch = glm::angleAxis(glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yaw = glm::angleAxis(glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat roll = glm::angleAxis(glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0));
    glm::mat4 tmp_rot = glm::mat4_cast(yaw * pitch * roll);

    //T R S from the father into T R S of the child
    absolute = glm::translate(translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale) *
               glm::translate(position_) * tmp_rot * glm::scale(glm::mat4(1.0f), scale_);

    updated_ = true;
}

void TransformComponent::UpdateRelativeMatrix(){
    glm::quat pitch = glm::angleAxis(glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yaw = glm::angleAxis(glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat roll = glm::angleAxis(glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0));
    glm::mat4 tmp_rot = glm::mat4_cast(yaw * pitch * roll);
    relative = glm::translate(position_) * tmp_rot * glm::scale(glm::mat4(1.0f), scale_);

    updated_ = true;
}
