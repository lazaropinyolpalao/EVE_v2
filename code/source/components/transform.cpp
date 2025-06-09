#include <transform.hpp>

// #### TRANSFORM COMPONENT ####

TransformComponent::TransformComponent() {
    changed_ = true;
    scale_ = glm::vec3(1.0f);
    rotation_ = glm::vec3(0.0f);
    position_ = glm::vec3(0.0f);
    relative = GetMatrix();
    absolute = glm::mat4();
}

glm::mat4 TransformComponent::GetMatrix() {
    if (changed_) {
        glm::quat pitch = glm::angleAxis(glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat roll = glm::angleAxis(glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0));
        glm::mat4 tmp_rot = glm::mat4_cast(yaw * pitch * roll);
        relative = glm::translate(position_) * tmp_rot * glm::scale(glm::mat4(1.0f), scale_);
        changed_ = false;
    }
    return relative;
}

TransformComponent* TransformComponent::SetScale(float x, float y, float z) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    scale_ = glm::vec3(x, y, z);

    return this;
}

TransformComponent* TransformComponent::SetScale(glm::vec3 scl) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    scale_ = scl;

    return this;
}

TransformComponent* TransformComponent::AddRotationX(float x, bool radians) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    if (radians) { x = glm::degrees(x); }
    rotation_.x += x;

    return this;
}

TransformComponent* TransformComponent::AddRotationY(float y, bool radians) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    if (radians) { y = glm::degrees(y); }
    rotation_.y += y;

    return this;
}

TransformComponent* TransformComponent::AddRotationZ(float z, bool radians) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    if (radians) { z = glm::degrees(z); }
    rotation_.z += z;

    return this;
}

TransformComponent* TransformComponent::AddRotation(float x, float y, float z, bool radians) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    if (radians) {
        x = glm::degrees(x);
        y = glm::degrees(y);
        z = glm::degrees(z);
    }
    rotation_.x += x;
    rotation_.y += y;
    rotation_.z += z;

    return this;
}

TransformComponent* TransformComponent::AddRotation(glm::vec3 rot, bool radians) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    if (radians) {
        rot.x = glm::degrees(rot.x);
        rot.y = glm::degrees(rot.y);
        rot.z = glm::degrees(rot.z);
    }
    rotation_.x += rot.x;
    rotation_.y += rot.y;
    rotation_.z += rot.z;


    return this;
}

TransformComponent* TransformComponent::SetTranslation(float x, float y, float z) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    position_ = glm::vec3(x, y, z);

    return this;
}

TransformComponent* TransformComponent::SetTranslation(glm::vec3 tsl) {
    if (this == nullptr) { return nullptr; }
    changed_ = true;
    position_ = tsl;

    return this;
}