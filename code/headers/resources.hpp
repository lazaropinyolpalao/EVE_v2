#ifndef __RESOURCES_H__
#define __RESOURCES_H__ 1

#include <memory>
#include <texture.hpp>
#include <tinyobj.hpp>
#include <cubemap.hpp>
#include <audio.hpp>


struct RenderingText {
	std::string text_;
	glm::vec3 color_;
	float scale_;
	float pos_x;
	float pos_y;

	RenderingText() {
		text_ = "";
		color_ = glm::vec3(0.0f, 0.0f, 0.0f);
		scale_ = 1.0f;
		pos_x = 0.0f;
		pos_y = 0.0f;
	}

	RenderingText(std::string text, float x, float y, glm::vec3 color, float s) {
		text_ = text;
		pos_x = x;
		pos_y = y;
		color_ = color;
		scale_ = s;
	}

	~RenderingText() {
		text_.clear();
	}
};

/**
 * @brief Different types of resources that can be used on the engine
 */
struct Resources {
	/** List of loaded textures */
	std::vector<std::shared_ptr<Texture>> textures_;
	/** List of loaded meshes */
	std::vector<std::shared_ptr<TinyObj>> meshes_;
	/** Cubemap of the scene */
	std::unique_ptr<Cubemap> cubemap_;
	/** Texts to render in the scene */
	std::vector<std::unique_ptr<RenderingText>> screen_texts_;
	/** Audio of the scene*/
	std::vector<std::unique_ptr<Audio>> audios_;

	//TODO:Remains to search in both functions for previously loaded 
	//elements and return a reference to them instead of loading another one

#ifdef RENDER_OPENGL
	/**
	 * @brief Loads a Texture and stores it in the resources list
	 *
	 * @param filepath Path to the texture file
	 *
	 * @return std::shared_ptr<Texture> A pointer to the loaded texture
	 */
	std::shared_ptr<Texture> addTexture(std::string filepath);

	/**
	 * @brief Loads a TinyObj mesh and stores it in the resources list
	 *
	 * @param filepath Path to the mesh file
	 *
	 * @return std::shared_ptr<TinyObj> A pointer to the loaded mesh
	 */
	std::shared_ptr<TinyObj> addMesh(std::string filepath);

	/**
	 * @brief Init resources like the cubemap mesh
	 *
	 */
	bool InitResources();
#endif
#ifdef RENDER_DIRECTX11
	/**
	* @brief Loads a Texture and stores it in the resources list
	*
	* @param Device to create the texture into
	* @param filepath Path to the texture file
	*
	* @return std::shared_ptr<Texture> A pointer to the loaded texture
	*/
	std::shared_ptr<Texture> addTexture(ID3D11Device* dev, std::string filepath);

	/**
	 * @brief Loads a TinyObj mesh and stores it in the resources list
	 *
	 * @param Device to create the texture into
	 * @param filepath Path to the mesh file
	 *
	 * @return std::shared_ptr<TinyObj> A pointer to the loaded mesh
	 */
	std::shared_ptr<TinyObj> addMesh(ID3D11Device* dev, ID3D11DeviceContext* devCon, std::string filepath);



	/**
	 * @brief Init resources like the cubemap mesh
	 *
	 */
	bool InitResources(ID3D11Device* dev);
#endif
	/**
	 * @brief Enable/Hide cubemap
	 */
	void toggleCubeMap();

	/**
	 * @brief Clear the resources from memory
	 * 
	 */
	void ClearResources();



	RenderingText* addTextToRender(std::string text, float x, float y, glm::vec3 color = glm::vec3(0.0f), float s = 1.0f);

	RenderingText* addTextToRender();

	Audio* addAudioFile(std::string filepath);

	std::shared_ptr<TinyObj> getMeshByName(std::string objfile);
	std::shared_ptr<Texture> getTextureByName(std::string texturefile);

};

#endif //__RESOURCES_H__
