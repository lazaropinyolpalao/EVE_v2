#ifndef __DEPTH_MAP_HPP__
#define __DEPTH_MAP_HPP__ 1

#ifdef RENDER_OPENGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#endif

#include <iostream>

/**
 * @brief Enum to determine the type of depthmap that will be generated
 */
enum class DepthMapType {
	kTypeQuad,
	kTypeCubeMap
};

/**
 * @brief Group of elements used to create and manage a depthmap for <B>light shadows</B>
 */
struct DepthMap {

	/** @brief Dimensions of the depthmap that will be used in width and height, that must be square and OPTIMALLY power of 2 */
	unsigned int dimensions_;
	/** @brief Wheter the depthmap buffers and texture are already loaded or not */
	bool loaded_;
	/**  @brief Opengl texture buffer id for the depthmap */
#ifdef RENDER_OPENGL
	GLuint depthMapTexture_;
	/** @brief Opengl frame buffer that the depthmap will use to render	*/
	GLuint depthMapFrameBufferObject_;
#endif
	/** @brief Type of depthmap that will be generated */
	DepthMapType type_;	
	
	DepthMap();
	DepthMap(unsigned int resolution, DepthMapType type);
	~DepthMap();

	/**
	 * @brief Generate the buffers and texture of the depthmap
	 * 
	 * @param resolution Resolution to generate the depthmap with
	 */
	void GenerateDepthMapBuffers(unsigned int resolution);

	/**
	 * @brief Set the viewport to the depthmap resolutions and bind the buffer of the depthmap.
	 */
	void SetBuffer();

	/**
	 * @brief Unbind the framebuffer of the depthmap.
	 */
	void UnsetBuffer();
	
};

#endif //__DEPTH_MAP_HPP__