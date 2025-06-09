#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__ 1

//GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

/**
 * @brief Transformation component that gives an object scale, rotation and translation
 */
struct TransformComponent {

	/** Absolute matrix, includes the transformation info from the parent objects */
	glm::mat4 absolute;
	/** Personal matrix of the transform component */
	glm::mat4 relative;

	/** Scale vector of the transform component */
	glm::vec3 scale_;
	/** Rotation vector of the transform component */
	glm::vec3 rotation_;
	/** Position vector of the transform component */
	glm::vec3 position_;
	/** Wheter the data of the transform component has changed or not, used to update the main matrix of the transform component */
	bool changed_;

	TransformComponent();

	/**
	 * @brief Gets the main matrix of the transform component
	 *
	 * @return glm::mat4 The main matrix of the transform component
	 */
	glm::mat4 GetMatrix();

	/**
	 * @brief Sets the scale of the transform component
	 *
	 * @param x Desired scale for the X axis
	 * @param y Desired scale for the Y axis
	 * @param z Desired scale for the Z axis
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetScale(float x, float y, float z);

	/**
	 * @brief Sets the scale of the transform component
	 *
	 * @param scl A vector with the desired scale for the X, Y and Z axis
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetScale(glm::vec3 scl);


	/**
	 * @brief Adds a rotation to the X axis of the transform component
	 *
	 * @param x Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationX(float x, bool radians = false);

	/**
	 * @brief Adds a rotation to the Y axis of the transform component
	 *
	 * @param y Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationY(float y, bool radians = false);

	/**
	 * @brief Adds a rotation to the Z axis of the transform component
	 *
	 * @param z Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationZ(float z, bool radians = false);

	/**
 * @brief Adds a rotation to the X, Y and Z axis of the transform component
 *
 * @param x Desired angle to add to the X axis
 * @param y Desired angle to add to the Y axis
 * @param z Desired angle to add to the Z axis
 * @param radians Whether the angle is given in radians or degrees
 *
 * @return TransformComponent* A pointer to the same modified TransformComponent
 */
	TransformComponent* AddRotation(float x, float y, float z, bool radians = false);

	/**
 * @brief Adds a rotation to the X, Y and Z axis of the transform component
 *
 * @param x Desired angle to add to the X axis
 * @param y Desired angle to add to the Y axis
 * @param z Desired angle to add to the Z axis
 * @param radians Whether the angle is given in radians or degrees
 *
 * @return TransformComponent* A pointer to the same modified TransformComponent
 */
	TransformComponent* AddRotation(glm::vec3 rot, bool radians = false);


	/**
	 * @brief Sets the position of the transform component
	 *
	 * @param x Desired position for the X axis
	 * @param y Desired position for the Y axis
	 * @param z Desired position for the Z axis
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetTranslation(float x, float y, float z);

	/**
	 * @brief Sets the position of the transform component
	 *
	 * @param tsl A vector with th desired position for the X, Y and Z axis
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetTranslation(glm::vec3 tsl);

};

#endif