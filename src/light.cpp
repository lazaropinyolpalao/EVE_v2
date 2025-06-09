#include "light.hpp"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

// #### DIRECTIONAL LIGHT ####
BaseLight::BaseLight() {

  visible_ = true;
  changed_ = true;

  position_ = glm::vec3(0.0f);
  direction_ = glm::vec3(0.0f);
  ambient_ = glm::vec3(1.0f);
  diffuse_ = glm::vec3(1.0f);
  specular_ = glm::vec3(1.0f);

  znear_ = 0.2f;
  zfar_ = 200.0f;

  left_ = -100.0f;
  right_ = 100.0f;
  bottom_ = -100.0f;
  top_ = 100.0f;

  fov_ = 90.0f;
  aspect_ratio_ = 1.0f;

}

BaseLight::~BaseLight() { }

void BaseLight::set_visible(bool visible) {
  visible_ = visible;
}

void BaseLight::set_position(float x, float y, float z) {
  position_ = glm::vec3(x, y, z);
}

void BaseLight::set_ambient(float r, float g, float b) {
  ambient_ = glm::vec3(r, g, b);
}

void BaseLight::set_diffuse(float r, float g, float b) {
  diffuse_ = glm::vec3(r, g, b);
}

void BaseLight::set_specular(float r, float g, float b) {
  specular_ = glm::vec3(r, g, b);
}

void BaseLight::set_near(float n) {
  znear_ = n;
}

void BaseLight::set_far(float f) {
  zfar_ = f;
}

void BaseLight::set_orthogonal(float l, float r, float t, float b) {
  left_ = l;
  right_ = r;
  top_ = t;
  bottom_ = b;
}

void BaseLight::set_fov(float fov) {
  fov_ = fov;
}

void BaseLight::set_aspect_ratio(float ar) {
  aspect_ratio_ = ar;
}

bool BaseLight::get_visible() {
  return visible_;
}

glm::vec3 BaseLight::get_position() {
  return position_;
}

glm::vec3 BaseLight::get_ambient() {
  return ambient_;
}

glm::vec3 BaseLight::get_diffuse() {
  return diffuse_;
}

glm::vec3 BaseLight::get_specular() {
  return specular_;
}

float BaseLight::get_near() {
  return znear_;
}

float BaseLight::get_far() {
  return zfar_;
}

float BaseLight::get_orthogonal_left() {
  return left_;
}

float BaseLight::get_orthogonal_right() {
  return right_;
}

float BaseLight::get_orthogonal_bottom() {
  return bottom_;
}

float BaseLight::get_orthogonal_top() {
  return top_;
}

float BaseLight::get_fov() {
  return fov_;
}

float BaseLight::get_aspect_ratio() {
  return aspect_ratio_;
}

DirectionalLight::DirectionalLight() {

  aspect_ratio_ = 1.0f;

  position_ = glm::vec3  { 0.0f,  0.0f,  0.0f };
  ambient_ = glm::vec3   { 0.25f, 0.25f, 0.25f };
  diffuse_ =  glm::vec3  { 1.0f,  1.0f,  1.0f };
  specular_ =  glm::vec3 { 1.0f,  1.0f,  1.0f };

  rotation_ = glm::mat4(1.0f);

  znear_ = 1.0f;
  zfar_ = 200.0f;

  pitch_ = 45.0f;
  yaw_ = 45.0f;

  UpdateProjection();
  UpdateView();
}

DirectionalLight::DirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float pitch, float yaw) {
  aspect_ratio_ = 1.0f;

  position_ = glm::vec3(0.0f, 0.0f, 0.0f);
  ambient_ = ambient;
  diffuse_ = diffuse;
  specular_ = specular;

  rotation_ = glm::mat4(1.0f);

  znear_ = 1.0f;
  zfar_ = 200.0f;

  yaw_ = yaw;
  pitch_ = pitch;

  UpdateProjection();
  UpdateView();
}

DirectionalLight::~DirectionalLight() {}

void DirectionalLight::UpdateProjection() {
  projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
  lightMatrix_ = projection_ * view_;
}

void DirectionalLight::UpdateView() {

  float X = glm::radians(pitch_);
  float Y = glm::radians(yaw_);

  direction_.x = cos(Y) * cos(X);
  direction_.y = sin(X);
  direction_.z = sin(Y) * cos(X);
  direction_ = glm::normalize(direction_);

  if(pitch_ == -90.0f){ 
    direction_.y = -0.95f;
    direction_.x = -0.025f;
    direction_.z = -0.025f;
  }

  view_ = glm::lookAt(position_,
    position_ + direction_,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  lightMatrix_ = projection_ * view_;
}

void DirectionalLight::set_yaw(float yaw) {
  yaw_ = yaw;
}

void DirectionalLight::set_pitch(float pitch) {
  pitch_ = pitch;
}

float DirectionalLight::get_yaw() {
  return yaw_;
}

float DirectionalLight::get_pitch() {
  return pitch_;
}


// #### POINT LIGHT ####

PointLight::PointLight() {

  aspect_ratio_ = 1.0;

  position_ = glm::vec3 { 0.0f, 10.0f, 0.0f };
  ambient_ = glm::vec3  { 1.0f,  1.0f, 1.0f };
  diffuse_ = glm::vec3  { 1.0f,  1.0f, 1.0f };
  specular_ = glm::vec3 { 1.0f,  1.0f, 1.0f };

  constant_ = 1.0f;
  linear_ = 0.075f;
  quadratic_ = 0.001f;

  range_ = 100.0f;

  znear_ = 0.1f;
  zfar_ = znear_ + range_;

  UpdateProjection();
  UpdateView();

}

PointLight::PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) {
  
  aspect_ratio_ = 1.0;
  position_ = position;
  ambient_ = ambient;
  diffuse_ = diffuse;
  specular_ = specular;
  constant_ = constant;
  linear_ = linear;
  quadratic_ = quadratic;

  range_ = 100.0f;

  znear_ = 2.0f;
  zfar_ = znear_ + range_;

  UpdateProjection();
  UpdateView();
}

PointLight::~PointLight() {}

void PointLight::UpdateProjection(){
  projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
}

void PointLight::UpdateView(){
  //Generate each of the 6 views and matrix used for every side of the cubemap shadow depthmap
  //The order will be right, left, top, down, front, back
  glm::vec3 pos = glm::vec3(position_.x, position_.y, position_.z);
  //Right
  lightMatrix_[0] = projection_ * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
  //Left
  lightMatrix_[1] = projection_ * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
  //Top
  lightMatrix_[2] = projection_ * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
  //Down
  lightMatrix_[3] = projection_ * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
  //Back
  lightMatrix_[4] = projection_ * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
  //Front
  lightMatrix_[5] = projection_ * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
}

void PointLight::set_constant(float constant) {
  constant_ = constant;
}

void PointLight::set_linear(float linear) {
  linear_ = linear;
}

void PointLight::set_quadratic(float quadratic) {
  quadratic_ = quadratic;
}

float PointLight::get_constant() {
  return constant_;
}

float PointLight::get_linear() {
  return linear_;
}

float PointLight::get_quadratic() {
  return quadratic_;
}

// #### SPOT LIGHT ####

SpotLight::SpotLight() {

  aspect_ratio_ = 1.0;

  position_ = glm::vec3 { 0.0f, 10.0f, 0.0f };
  ambient_ = glm::vec3  { 1.0f,  1.0f, 1.0f };
  diffuse_ = glm::vec3  { 1.0f,  1.0f, 1.0f };
  specular_ = glm::vec3 { 1.0f,  1.0f, 1.0f };

  pitch_ =  -90.0f;
  yaw_ =  0.0f;

  cut_off_ = 0.75f;
  outer_cut_off_ = 0.5f;
  constant_ = 1.0f;
  linear_ = 0.0025f;
  quadratic_ = 0.0001f;

  fov_ = 90.0f;
  range_ = 100.0f;

  rotation_ = glm::mat4(1.0f);

  left_ = -100.0f;
  right_ = 100.0f;
  bottom_ = -100.0f;
  top_ = 100.0f;

  znear_ = 0.2f;
  zfar_ = 150.0f;

  UpdateProjection();
  UpdateView();

}

SpotLight::SpotLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
  float cut_off, float outer_cut_off, float constant, float linear, float quadratic, float pitch, float yaw) {

  aspect_ratio_ = 1.0;

  position_ = position;
  ambient_ = ambient;
  diffuse_ = diffuse;
  specular_ = specular;
  cut_off_ = cut_off;
  outer_cut_off_ = outer_cut_off;
  constant_ = constant;
  linear_ = linear;
  quadratic_ = quadratic;

  range_ = 100.0f;

  rotation_ = glm::mat4(1.0f);

  left_ = -100.0f;
  right_ = 100.0f;
  bottom_ = -100.0f;
  top_ = 100.0f;

  znear_ = 2.0f;
  zfar_ = 150.0f;

  pitch_ = pitch;
  yaw_ = yaw;

  UpdateProjection();
  UpdateView();
}

SpotLight::~SpotLight() {}


void SpotLight::UpdateProjection() {
    projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
    lightMatrix_ = projection_ * view_;
}

void SpotLight::UpdateView() {

  float X = glm::radians(pitch_);
  float Y = glm::radians(yaw_);

  direction_.x = cos(Y) * cos(X);
  direction_.y = sin(X);
  direction_.z = sin(Y) * cos(X);
  direction_ = glm::normalize(direction_);

  if (pitch_ == -90.0f) {
    direction_.y = -0.95f;
    direction_.x = -0.025f;
    direction_.z = -0.025f;
  }

  view_ = glm::lookAt(position_,
    position_ + direction_,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  lightMatrix_ = projection_ * view_;
}

void SpotLight::set_yaw(float yaw) {
  yaw_ = yaw;
}

void SpotLight::set_pitch(float pitch) {
  pitch_ = pitch;
}

void SpotLight::set_cut_off(float cut_off) {
  cut_off_ = cut_off;
}

void SpotLight::set_outer_cut_off(float outer_cut_off) {
  outer_cut_off_ = outer_cut_off;
}

void SpotLight::set_constant(float constant) {
  constant_ = constant;
}

void SpotLight::set_linear(float linear) {
  linear_ = linear;
}

void SpotLight::set_quadratic(float quadratic) {
  quadratic_ = quadratic;
}

float SpotLight::get_yaw() {
  return yaw_;
}

float SpotLight::get_pitch() {
  return pitch_;
}

float SpotLight::get_cut_off() {
  return cut_off_;
}

float SpotLight::get_outer_cut_off() {
  return outer_cut_off_;
}

float SpotLight::get_constant() {
  return constant_;
}

float SpotLight::get_linear() {
  return linear_;
}

float SpotLight::get_quadratic() {
  return quadratic_;
}

// #### ADDITIVE LIGHTS ####

DirectionalLight* Lights::addDirectionalLight() {
  directional_.emplace_back(std::make_unique<DirectionalLight>());
  return directional_.back().get();
}

DirectionalLight* Lights::addDirectionalLight(DirectionalLight& directional) {
  directional_.emplace_back(std::make_unique<DirectionalLight>(
    directional.ambient_,
    directional.diffuse_,
    directional.specular_,
    directional.pitch_,
    directional.yaw_)
  );
  return directional_.back().get();  
}

DirectionalLight* Lights::addDirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float pitch, float yaw) {
  directional_.emplace_back(std::make_unique<DirectionalLight>(ambient, diffuse, specular, pitch, yaw));    
  return directional_.back().get();
}

PointLight* Lights::addPointLight() {
  point_.emplace_back(std::make_unique<PointLight>());
  return point_.back().get();
}

PointLight* Lights::addPointLight(PointLight& point) {
  point_.emplace_back(std::make_unique<PointLight>(
    point.position_, 
    point.ambient_, 
    point.diffuse_, 
    point.specular_, 
    point.constant_, 
    point.linear_, 
    point.quadratic_)
  );
  return point_.back().get();
}

PointLight* Lights::addPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) {
  point_.emplace_back(std::make_unique<PointLight>(position, ambient, diffuse, specular, constant, linear, quadratic));
  return point_.back().get();
}

SpotLight* Lights::addSpotLight() {
  spot_.emplace_back(std::make_unique<SpotLight>());
  return spot_.back().get();
}

SpotLight* Lights::addSpotLight(SpotLight& spot) {
  spot_.emplace_back(std::make_unique<SpotLight>(
    spot.position_, 
    spot.ambient_, 
    spot.diffuse_, 
    spot.specular_, 
    spot.cut_off_, 
    spot.outer_cut_off_, 
    spot.constant_, 
    spot.linear_, 
    spot.quadratic_, 
    spot.pitch_, 
    spot.yaw_)
  );
  return spot_.back().get();
}

SpotLight* Lights::addSpotLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float cut_off, float outer_cut_off, float constant, float linear, float quadratic, float pitch, float yaw) {
  spot_.emplace_back(std::make_unique<SpotLight>(position, ambient, diffuse, specular, cut_off, outer_cut_off, constant, linear, quadratic, pitch, yaw));
  return spot_.back().get();
}