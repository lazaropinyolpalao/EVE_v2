#ifndef __render_system_opengl_H__
#define __render_system_opengl_H__ 1

#include <render_system.hpp>

#include <program.hpp>
#include <depth_map.hpp>
#include <deferred_framebuffer.hpp>
#include <light.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H  


/**
 * @brief Struct to hold each letter of the freetype font
 */
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};


class RenderSystemOpenGL : public RenderSystem {
public:
  RenderSystemOpenGL(int window_w, int window_h);
  ~RenderSystemOpenGL();

  virtual void Render(ComponentManager* comp);
  virtual void Update();
	Window* getWindow();

	void ResetResources();
	
private:



	void ResizeBuffers(int w, int h);

	RenderSystemOpenGL() = delete;
	/** Window of the engine */
	std::unique_ptr<Window> window_;

	//## TEXT RENDERING
	/**
	 * @brief Init the text rendering sources and map the characters
	 */
	bool InitTextResources();
	bool InitLetterCharacters();
	void ResetTextResources();

	void RenderText(std::string text, float screen_x, float screen_y, float scale, glm::vec3 color);

	//## FORWARD RENDERING
	/**
	* @brief Render the scene cubemap
	*/
	void render_scene_cubemap(ComponentManager* comp);
	/**
	 * @brief Render only the elements with their textures associated
	 * 
	 * @param comp ComponentManager to get the data from
	 * @param prog Program to use in the rendering of the scene
	 */
	void render_elements_with_texture(ComponentManager* comp, Program* prog);
	/**
	 * @brief Render only the elements for the shadow depthmap
	 *
	 * @param comp ComponentManager to get the data from
	 * @param prog Program to use in the rendering of the scene
	 */
	void render_elements_depthmap(ComponentManager* comp, Program* prog);
	/**
	 * @brief Render the elements with a directionallight
	 *
	 * @param comp ComponentManager to get the data from
	 * @param prog Program to use in the rendering of the scene
	 * @param directional The DirectionalLight to draw with
	 */
	void render_light_elements(ComponentManager* comp, Program* prog, DirectionalLight* directional);

	/**
	* @brief Render the elements with a spotlight
	*
	* @param comp ComponentManager to get the data from
	* @param prog Program to use in the rendering of the scene
	* @param spotlight The SpotLight to draw with
	*/
	void render_light_elements(ComponentManager* comp, Program* prog, SpotLight* spotlight);

	/**
	* @brief Render the elements with a pointlight
	*
	* @param comp ComponentManager to get the data from
	* @param prog Program to use in the rendering of the scene
	* @param pointlight The PointLight to draw with
	*/
	void render_light_elements(ComponentManager* comp, Program* prog, PointLight* pointlight);
	/**
	 * @brief Forward rendering method
	 * 
	 * @param comp ComponentManager to get the data from
	 */
	void ForwardRendering(ComponentManager* comp);


	/** Program for drawing elements with only ambient and textures */
	std::unique_ptr<Program> render_elements_with_texture_;

	/** Program for rendering directional and spotlight shadows */
	std::unique_ptr<Program> render_directional_and_spotlight_shadows_;

	/** Program for rendering elements with directional light and shadow */
	std::unique_ptr<Program> render_elements_directional_light_;
	/** Program for rendering elements with spotlight and shadow */
	std::unique_ptr<Program> render_elements_spotlight_;

	/** Program for rendering pointlight shadows */
	std::unique_ptr<Program> render_pointlight_shadows_;
	/** Program for rendering elements with pointlight and shadow */
	std::unique_ptr<Program> render_elements_pointlight_;

	/** Depthmap used in both directional and spotlight shadows */
	std::unique_ptr<DepthMap> depthmap_directional_and_spotlight_shadows_;

	/** Depthmap used to render pointlight shadows */
	std::unique_ptr<DepthMap> depthmap_pointlight_shadows_;

	//## DEFERRED RENDERING

	/**
	* @brief Sets the DirectionalLight data to the corresponding shader and renders it
	*
	* @param prog Program to use in the rendering of the scene
	* @param directional The DirectionalLight to draw with
	*/
	void render_deferred_light(Program* prog, DirectionalLight* directional);
	/**
	 * @brief Sets the SpotLight data to the corresponding shader and renders it
	 *
	 * @param prog Program to use in the rendering of the scene
	 * @param spotlight The SpotLight to draw with
	 */
	void render_deferred_light(Program* prog, SpotLight* spotlight);

	/**
	 * @brief Sets the PointLight data to the corresponding shader and renders it
	 *
	 * @param prog Program to use in the rendering of the scene
	 * @param pointlight The PointLight to draw with
	 */
	void render_deferred_light(Program* prog, PointLight* pointlight);

	/**
	* @brief Render only the elements with their textures associated
	*
	* @param comp ComponentManager to get the data from
	* @param prog Program to use in the rendering of the scene
	*/
	void render_deferred_elements_without_light(ComponentManager* comp, Program* prog);

	/**
	* @brief Deferred rendering method.
	*/
	void DeferredRendering(ComponentManager* comp);

	std::unique_ptr<DeferredFramebuffer> deferred_framebuffer_;

	/** Program for drawing elements on deferred rendering */
	std::unique_ptr<Program> deferred_rendering_geometry_program_;

	/** Program for drawing lights on deferred rendering */
	std::unique_ptr<Program> deferred_rendering_directionallight_program_;

	/** Program for drawing lights on deferred rendering */
	std::unique_ptr<Program> deferred_rendering_spotlight_program_;

	/** Program for drawing lights on deferred rendering */
	std::unique_ptr<Program> deferred_rendering_pointlight_program_;

	/** Program for drawing elements on deffered rendering without light */
	std::unique_ptr<Program> deferred_rendering_elements_with_texture_program_;

	std::shared_ptr<TinyObj> last_loaded_mesh = nullptr;

	/** Freetype content */
	bool free_type_init = false;
	std::map<char, Character> characters_;
	glm::mat4 text_proj;
	unsigned int text_render_VAO, text_render_VBO, text_render_IBO;
	std::unique_ptr<Program> render_text_program_;
};

#endif //__render_system_opengl_H__
