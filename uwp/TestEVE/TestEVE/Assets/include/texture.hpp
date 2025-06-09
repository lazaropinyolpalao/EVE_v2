#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__ 1

#include <d3d11.h>
#include <string>
#include <memory>

#ifdef RENDER_DIRECTX11
	#include <wrl.h>
#endif

#ifdef RENDER_OPENGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#endif

/**
 * @brief Structure of a texture that can be rendered on a mesh
 */
struct Texture {
	/** Texture width */
	int width_;
	/** Texture height */
	int height_;
	/** Number of channels of the texture */
	int channels_;
	int desired_channels_;
#ifdef RENDER_OPENGL
	/** Opengl id of the texture */
	GLuint texture_id_;
#endif 
#ifdef RENDER_DIRECTX11
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view_;
#endif 
	/** Whether the texture has been created or not */
	bool loaded_;
	/** Path of the texture */
	std::string src_;

	Texture();
	~Texture();


#ifdef RENDER_OPENGL
	/**
	 * @brief Creates an Opengl texture
	 * 
	 * @param filepath Path of the texture to load
	 * 
	 * @return bool True if the texture is loaded successfully, False if not. 
	 */
	bool LoadTexture(std::string filepath);


	/**
	 * @brief Creates an Opengl texture with params for a cubemap
	 *
	 * @param filepath Path of the texture to load
	 *
	 * @return bool True if the texture is loaded successfully, False if not.
	 */
	bool LoadCubemapTexture(std::string filepath);
#endif
#ifdef RENDER_DIRECTX11
	/**
	* @brief Creates a directx11 texture
	*
	* @param Device to create the texture into
	* @param Route to the texture source
	* 
	* @return bool True if the texture is loaded successfully, false if not
	*/
	bool LoadTexture(ID3D11Device* dev, std::string src);



	/**
	 * @brief Creates an Opengl texture with params for a cubemap
	 *
	 * @param Device to create the texture into
	 * @param filepath Path of the texture to load
	 *
	 * @return bool True if the texture is loaded successfully, False if not.
	 */
	bool LoadCubemapTexture(ID3D11Device* dev, std::string filepath);
#endif

	/**
	 * @brief Frees a texture and deletes its Opengl id
	 * 
	 * @return bool True
	 */
	bool FreeTexture();

};

#endif //__TEXTURE_HPP__