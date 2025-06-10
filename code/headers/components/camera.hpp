#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__ 1 

//GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

#include <defines.hpp>
#include <window.hpp>

/**
 * @brief Enumeration of the different camera modes
 */
enum class CameraMode {
	kPerspective,
	kOrthographic
};

/**
 * @brief Camera component that provides a projection of the scene
 */
struct CameraComponent {
	//Friend declaration to update transformations 
	// through the tree component at the component manager
	friend struct ComponentManager;

	/** Whether the camera is active or not */
	bool is_active_;


protected:
	//Shared elements
	/** Active camera mode */
	CameraMode mode_;

	//X, Y, Z
	/** Angle of rotation on the X axis */
	float pitch_;
	/** Angle of rotation on the Y axis */
	float yaw_;
	/** Angle of rotation on the Z axis */
	float roll_;

	/** Position of the camera */
	glm::vec3 position_;
	/** Rotation of the camera */
	//glm::mat4 rotation_;

	/** Forward direction of the camera */
	//glm::vec3 forward_dir_;
	/** Right direction of the camera */
	//glm::vec3 right_dir_;
	/** Up direction of the camera */
	//glm::vec3 up_dir_;

	/** Projection matrix of the camera */
	glm::mat4 projection_;
	/** View matrix of the camera */
	glm::mat4 view_;

	/** Nearest distance where the camera will start its projection */
	float znear_;
	/** Farest distance where the camera will stop its projection */
	float zfar_;

	//Orthographic
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
	/** Field of view of the perspective camera */
	float aspect_ratio_;

	/** Camera speed*/
	float speed_;
	
public:
	CameraComponent();
	CameraComponent(CameraMode mode);

	/**
	 * @brief Sets the camera as an orthographic camera
	 *
	 * @param l Left expansion of the orthographic camera
	 * @param r Right expansion of the orthographic camera
	 * @param b Bottom expansion of the orthographic camera
	 * @param t Top expansion of the orthographic camera
	 * @param zn Near distance of the camera
	 * @param zf Far distance of the camera
	 *
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* SetOrthographic(float l, float r, float b, float t, float zn, float zf);

	/**
	 * @brief Sets the camera as a perspective camera
	 *
	 * @param f Field of view of the perspective camera
	 * @param ar Aspect ration of the perspective camera
	 * @param zn Near distance of the camera
	 * @param zf Far distance of the camera
	 *
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* SetPerspective(float f, float ar, float zn, float zf);

	/**
	 * @brief Updates the values of the camera when receives an input
	 *
	 * @param i Input to process
	 * @param dt Elapsed time between one frame and another
	 * @param speed Camera movement speed
	 *
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* UpdateCamera(Input i, float dt);

	// Setters
	void set_mode(CameraMode m);
	void set_pitch(float pitch);
	void set_yaw(float yaw);
	void set_roll(float roll);
	void set_position(float x, float y, float z);
	void set_near(float n);
	void set_far(float f);
	void set_orthographic_left(float l);
	void set_orthographic_right(float r);
	void set_orthographic_bottom(float b);
	void set_orthographic_top(float t);
	void set_orthographic_limits(float l, float r, float b, float t);
	void set_fov(float fov);
	void set_aspect_ratio(float ar);
	void set_speed(float s);

	// Getters

	CameraMode get_mode();
	float get_pitch();
	float get_yaw();
	float get_roll();
	float get_near();
	float get_far();
	float get_orthographic_left();
	float get_orthographic_right();
	float get_orthographic_bottom();
	float get_orthographic_top();
	float get_fov();
	float get_aspect_ratio();
	float get_speed();

	glm::vec3 get_position();
	glm::mat4 get_view();
	glm::mat4 get_projection();
};

#endif
