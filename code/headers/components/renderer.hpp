#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__ 1

#include <vector>
#include <memory>
#include <tinyobj.hpp>

/**
 * @brief Renderer component that allows an entity to be drawn
 */
struct RendererComponent {
	//Friend declaration to update transformations 
	// through the tree component at the component manager
	friend struct ComponentManager;

	/** Whether the renderer component has a mesh assigned */
	bool isInit_;
	/** Whether the lights of the scene affects the object */
	bool needs_light_;
	/** Whether the object projects shadows */
	bool casts_shadows_;
	/** Whether the object shows shadows of other objects */
	bool receives_shadows_;

	/** Mesh of the renderer component */
	std::shared_ptr<TinyObj> mesh_;
	/** Different textures that can render the renderer component */
	std::vector<std::shared_ptr<Texture>> textures_;

	RendererComponent();

	/**
	 * @brief Assigns a mesh to the renderer component
	 *
	 * param mesh Mesh that will be assigned to the renderer component
	 *
	 * @return RendererComponent* A pointer to the same modified RendererComponent
	 */
	RendererComponent* Init(std::shared_ptr<TinyObj> mesh);


	/**
	 * @brief Adds a new texture to the renderer component
	 *
	 * @param texture New Texture to render
	 *
	 * @return RendererComponent* A pointer to the same modified RendererComponent
	 */
	RendererComponent* AddTexture(std::shared_ptr<Texture> texture);

	/**
	 * @brief Changes the mesh that will be rendered
	 *
	 * @param new_mesh New mesh that will be rendered
	 *
	 * @return RendererComponent* A pointer to the same modified RendererComponent
	 */
	RendererComponent* ChangeMesh(std::shared_ptr<TinyObj> new_mesh);

};

#endif