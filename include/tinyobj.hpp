#ifndef __TINYOBJ_HPP__
#define __TINYOBJ_HPP__	1


#include <vector>
#include <string>

#include <vertex.hpp>
#include <texture.hpp>

#include <iostream>
#include <map>
#include <algorithm>
#ifdef RENDER_DIRECTX11
	#include <wrl.h>
#endif

#ifdef RENDER_OPENGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#endif

//If the number of vertices is higher than this, it doesn't use indexes
//This is due to the high load times that uses to avoid repeated info
const unsigned int kVertexCountLimit = 50000;

/**
 * @brief Structure that represents a mesh in the scene
 */
struct TinyObj {

	/** Vector of the vertices of the mesh */
	std::vector<Vertex> vertices_;
	/** Vector of indexes to render the mesh */
	std::vector<unsigned int> indexes_;

#ifdef RENDER_OPENGL
	/** Opengl Virtual Buffer Object identifier */
	unsigned int virtual_buffer_object_;
	/** Opengl Virtual Array Object identifier */
	unsigned int virtual_array_object_;
	/** Opengl Index Buffer Object identifier */
	unsigned int index_buffer_object_;
#endif

#ifdef RENDER_DIRECTX11
	/** DirectX vertex buffer */
	Microsoft::WRL::ComPtr <ID3D11Buffer> g_pVertexBuffer;
	/** DirectX index buffer*/
	Microsoft::WRL::ComPtr <ID3D11Buffer> g_pIndexBuffer;
#endif

	/** Mesh name */
	std::string name_;
	/** Path of the loaded mesh */
	std::string full_path_;

	/** Check to optimize the mesh reading and loading */
	bool bigger_than_optimazable_;
	
	/** Whether if the buffers of the mesh have been initialized */
	bool isInit_;
	
	/** Culling Type: 0 - Front, 1 - Back, 2 - Both */
	unsigned char cull_type_;

	/**
	 * @brief Loads a mesh from a file
	 * 
	 * @param inputfile Path to file to read mesh from
	 */
	void LoadObj(std::string inputfile);

	TinyObj();
	~TinyObj();

	#ifdef RENDER_OPENGL
	void InitBuffer();
	#endif
	#ifdef RENDER_DIRECTX11
	void InitBuffer(ID3D11Device* dev, ID3D11DeviceContext* devCon);
	#endif
};

#endif //__TINYOBJ_HPP__