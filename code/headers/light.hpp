#ifndef __LIGHT_HPP__
#define __LIGHT_HPP__ 1

#include <vector>
#include <memory>
#include <defines.hpp>
#include <glm/glm.hpp>

// ### BASE LIGHT WITH COMMON ELEMENTS FOR ALL THE LIGHTS ### //
/**
 * @brief Base struct with shared elements between all the light types
 */
struct BaseLight {

  /** Whether the light is active or not */
  bool visible_;
  /** Whether the light data has changed or not, used to mark the light data that needs to be updated on the shader uniforms */
  bool changed_;

  /** Position on the scene of the light */
  glm::vec3 position_;
  /** Direction vector of the light */
  glm::vec3 direction_;
  /** Ambient values of the light */
  glm::vec3 ambient_;
  /** Diffuse values of the light */
  glm::vec3 diffuse_;
  /** Specular values of the light */
  glm::vec3 specular_;
  
  /** Near distance where the light shadow will start rendering */
  float znear_;
  /** far distance where the light shadow will stop rendering */
  float zfar_;

  //Orthogonal
  /** Left expansion of the orthographic camera */
  float left_;
  /** Right expansion of the orthographic camera */
  float right_;
  /** Bottom expansion of the orthographic camera */
  float bottom_;
  /** Top expansion of the orthographic camera */
  float top_;

  //Perspective
  /** Field of view of the perspective camera */
  float fov_;
  /** Aspect ratio of the perspective camera */
  float aspect_ratio_;

  BaseLight();
  ~BaseLight();

  // Setters
  void set_visible(bool visible);

  void set_position(float x, float y, float z);
  void set_ambient(float r, float g, float b);
  void set_diffuse(float r, float g, float b);
  void set_specular(float r, float g, float b);

  void set_near(float n);
  void set_far(float f);

  void set_orthogonal(float l, float r, float b, float t);
  void set_fov(float fov);
  void set_aspect_ratio(float ar);

  // Getters
  bool get_visible();

  glm::vec3 get_position();
  glm::vec3 get_ambient();
  glm::vec3 get_diffuse();
  glm::vec3 get_specular();

  float get_near();
  float get_far();

  float get_orthogonal_left();
  float get_orthogonal_right();
  float get_orthogonal_bottom();
  float get_orthogonal_top();

  float get_fov();
  float get_aspect_ratio();

};

// #### DIRECTIONAL LIGHT ####
/**
 * @brief Light without given position, that ilumminates all the elements of the scene at the same angle, like the sun would. <B>It's shadows use an orthographic camera</B>
 */
struct DirectionalLight : BaseLight {

  /** Matrix representing the transformation of the light */
  glm::mat4 lightMatrix_;
  /** Projection matrix of the camera light */
  glm::mat4 projection_;
  /** View matrix of the light */
  glm::mat4 view_;

  /** Yaw rotation of the light */
  float yaw_;
  /** Pitch rotation of the light */
  float pitch_;
  /** Rotation matrix of the light */
  glm::mat4 rotation_;

  DirectionalLight();
  DirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float pitch = 0.0f, float yaw = 0.0f);
  ~DirectionalLight();

  /** Regenerates the projection matrix of the light if the values have changed */
  void UpdateProjection();
  /** Regenerates the view matrix of the light if the values have changed */
  void UpdateView();

  // Setters
  void set_yaw(float yaw);
  void set_pitch(float pitch);

  // Getters
  float get_yaw();
  float get_pitch();

};

// #### POINT LIGHT ####
/**
 * @brief Point of light that lights all the elements close enough to the point, like a lightbulb would
 */
struct PointLight : BaseLight{
  /** Projection matrix of the camera light */
  glm::mat4 projection_;
  /** Different matrix of each of the 6 sides of the pointlight. The order will be <B>right, left, top, down, front, back</B> */
  glm::mat4 lightMatrix_ [6];
  
  /** Constant value of the light */
  float constant_;
  /** Linear value of the light */
  float linear_;
  /**  */
  float quadratic_;

  /** Range of effect of the light */
  float range_;

  PointLight();
  PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);
  ~PointLight();

  /**
   * @brief Regenerates the projection matrix of the light if the values have changed.
   */
  void UpdateProjection();

  /**
   * @brief Regenerates the view matrix of the light if the values have changed.
   */
  void UpdateView();

  // Setters
  void set_constant(float constant);
  void set_linear(float linear);
  void set_quadratic(float quadratic);

  // Getters
  float get_constant();
  float get_linear();
  float get_quadratic();

};

// #### SPOT LIGHT ####
/** @brief Light based on a focus-like type, that illuminates a directed place light a torch would  */
struct SpotLight : BaseLight{

  /** Projection matrix of the camera light */
  glm::mat4 lightMatrix_;
  /** Projection matrix of the camera light */
  glm::mat4 projection_;
  /** View matrix of the light */
  glm::mat4 view_;

  /** Yaw rotation of the light */
  float yaw_;
  /** Pitch rotation of the light */
  float pitch_;
  /** Rotation matrix of the light */
  glm::mat4 rotation_;

  /** Cut off value of the spot light */
  float cut_off_;
  /** Outer cut off value of the spot light */
  float outer_cut_off_;
  /** Constant value of the spot light */
  float constant_;
  /** Linear value of the spot light */
  float linear_;
  /** Quadratic value of the spot light */
  float quadratic_;

  float range_;

  SpotLight();
  SpotLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
    float cut_off, float outer_cutt_off, float constant, float linear, float quadratic, float pitch = 0.0f, float yaw = 0.0f);
  ~SpotLight();

  /**
   * @brief Regenerates the projection matrix of the light if the values have changed.
   */
  void UpdateProjection();

  /**
   * @brief Regenerates the view matrix of the light if the values have changed.
   */
  void UpdateView();

  // Setters
  void set_yaw(float yaw);
  void set_pitch(float pitch);
  void set_cut_off(float cut_off);
  void set_outer_cut_off(float outer_cut_off);
  void set_constant(float constant);
  void set_linear(float linear);
  void set_quadratic(float quadratic);

  // Getters
  float get_yaw();
  float get_pitch();
  float get_cut_off();
  float get_outer_cut_off();
  float get_constant();
  float get_linear();
  float get_quadratic();

};

// #### ADDITIVE LIGHTS ####
/**
 * @brief Struct that holds all the lights of the scene and controls the right usage and number of them
 */
struct Lights {

  /** Vector of directional lights of the scene */
  std::vector<std::unique_ptr<DirectionalLight>> directional_;
  /** Vector of point lights of the scene */
  std::vector<std::unique_ptr<PointLight>> point_;
  /** Vector of spot lights of the scene */
  std::vector<std::unique_ptr<SpotLight>> spot_;

  Lights() { };

  ~Lights() { 
    for (unsigned int i = 0; i < directional_.size(); ++i) {
      directional_[i].reset();
    }
    directional_.clear();

    for (unsigned int i = 0; i < point_.size(); ++i) {
      point_[i].reset();
    }
    point_.clear();

    for (unsigned int i = 0; i < spot_.size(); ++i) {
      spot_[i].reset();
    }
    spot_.clear();
   };

  /**
   * @brief Add a default directional light to the scene.
   */
  DirectionalLight* addDirectionalLight();

  /**
   * @brief Add a directional light to the scene.
   * 
   * @param light Directional light to get values from
   */
  DirectionalLight* addDirectionalLight(DirectionalLight& light);
  
  /**
   * @brief Add a directional light to the scene.
   *
   * @param ambient Ambient values to create the new light
   * @param diffuse Diffuse values to create the new light
   * @param specular Specular values to create the new light
   * @param pitch Pitch angle value to create the new light
   * @param yaw Yaw angle to create the new light
   */
  DirectionalLight* addDirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float pitch, float yaw);

  /**
  * @brief Add a default point light to the scene.
  */
  PointLight* addPointLight();

  /**
  * @brief Add a point light to the scene.
  *
  * @param light Point light to get values from
  */
  PointLight* addPointLight(PointLight& light);

  /**
   * @brief Add a point light to the scene.
   *
   * @param position Position values to create the new light
   * @param ambient Ambient values to create the new light
   * @param diffuse Diffuse values to create the new light
   * @param specular Specular values to create the new light
   * @param constant Constant value to create the new light
   * @param linear Linear value to create the new light
   * @param quadratic Quadratic value to create the new light
   */
  PointLight* addPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);

  /**
   * @brief Add a default spot light to the scene.
   */
  SpotLight* addSpotLight();

  /**
   * @brief Add a spot light to the scene.
   *
   * @param light Spot light to get values from
   */
  SpotLight* addSpotLight(SpotLight& light);

  /**
   * @brief Add a spot light to the scene.
   *
   * @param position Position values to create the new light
   * @param ambient Ambient values to create the new light
   * @param diffuse Diffuse values to create the new light
   * @param specular Specular values to create the new light
   * @param cut_off Cut off values to create the new light
   * @param outer_cut_off Outer cut off values to create the new light
   * @param constant Constant value to create the new light
   * @param linear Linear value to create the new light
   * @param quadratic Quadratic value to create the new light
   * @param pitch Pitch angle value to create the new light
   * @param yaw Yaw angle to create the new light
   */
  SpotLight* addSpotLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float cut_off, float outer_cut_off, float constant, float linear, float quadratic, float pitch, float yaw);

};

#endif // __LIGHT_HPP__