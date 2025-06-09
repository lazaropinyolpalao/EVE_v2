#ifndef __IMGUI_FUNCTIONS_HPP__
#define __IMGUI_FUNCTIONS_HPP__  1

//#include <engine.hpp>

#include <component_system.hpp>
#ifdef RENDER_OPENGL
  #include <render_system_opengl.hpp>
#endif
#ifdef RENDER_DIRECTX11
  #include <render_system_directx11.hpp>
#endif
#include <scene_manager.hpp>

/**
 * @brief Group of functions to manage the resources of the engine through imgui on runtime
 */
struct ImguiFunctions {

  /**
   * @brief Display information about the window execution, in terms of resolution, FPS and such
   * 
   * @param win std::optional Of the window that will be referenced to
   * @param comp ComponentManager pointer to where the data will be taken from
   */
  static void DisplayWindowData(Window* win, ComponentManager* comp);

  /**
   * @brief Displays a texture buffer of OpengGL into a ImGui Window for debug purposes
   * 
   * @param text Texture object to load image from
   */
  static void DisplayTexture(Texture* text);

  /**
   * @brief Displays a menu to select and load a new texture into the scene
   * 
   * @param comp ComponentManager where to get the textures from
   */
  static void DisplayLoadTexture(RenderSystem* render);

  /**
   * @brief Displays a menu to select and load a new mesh into the scene
   * 
   * @param com ComponentManager where to get the meshes from
   */
  static void DisplayLoadMesh(RenderSystem* render);

  /**
   * @brief Display a window listing all the resources loaded into the program, like textures, lights and meshes
   * 
   * @param comp Component manager to get the resources from
   */
  static void DisplayResourceList(ComponentManager* comp, RenderSystem* render);

  /**
   * @brief Displays the hierarchy graph of the elements in the scene in a tree format, and gives access to it's components in real time
   *
   * @param comp Component manager to get the resources from
   */
  static void DisplaySceneGraph(ComponentManager* comp);

  /**
   * @brief Displays a window to create a new entity into the component manager
   * 
   * @param comp Component manager to create the new entity into
   */
  static void DisplayNewEntity(ComponentManager* comp);

  /**
   * @brief Recursively draws each branch and leaf of the graph scene
   * 
   * @param tree_comps_ std::vector of TreeComponents that the tree node will built from
   * @param entity Which id to draw it's tree branches and leafs
   */
  static void DrawTree(std::vector<component_node<TreeComponent>>* tree_comps_, size_t entity);
  
  /**
   * @brief Display a window of components of a selected entity
   * 
   * @param comp Component manager to get the entity components from
   */
  static void DisplayEntityComponents(ComponentManager* comp, RenderSystem* renderer);

  /**
   * @brief Display a window to add a texture 
   * 
   * @param comp Component manager to get the data from and into
   * @param renderer Which renderer component to add the texture if done
   */
  static void DisplayAddTextureToRenderer(ComponentManager* comp, RendererComponent* renderer, RenderSystem* render_sys);

  /**
   * @brief Display a window with the scenes currently saved and it's data
   * 
   * @param e Engine reference to take the info from
   */
  static void DisplayScenes(SceneManager* sm);

  /*
   * @brief Display a window to edit the details of a scene
   * 
   * @param e Engine to take data from and into
   * @param scene_id Which scene to edit
  static void DisplayEditSceneDetails(Engine* e, unsigned int scene_id);
  */

  /**
   * @brief Display a window to confirm the delete of a scne
   * 
   * @param e Engine to take data from and into
   */
  static void DisplayDeleteScene(SceneManager* sm);

  /**
   * @brief Display the dialog to create a new scene
   * 
   * @param e Engine to take data from and into
   */
  static void DisplayNewScene(ComponentManager* comp);

  /**
   * @brief Display the dialog to save the scene
   *
   * @param e Engine to take data from and into
   */
  static void DisplaySaveNewScene(ComponentManager* cm, RenderSystem* rs, SceneManager* sm);

  /**
   * @brief Display the dialog to load a scene
   *
   * @param e Engine to take data from and into
   */
  static void DisplayLoadScene(ComponentManager* cm, RenderSystem* rs, SceneManager* sm);


  /**
   * @brief Display the top screen bar with access to diferent functions
   * 
   * @param win Window to display the bar in
   */
  static void DisplayMenuBar(Window* win, SceneManager* sm);

  /**
   * @brief Reset all static variables that set the displaying menus
   */
  static void ResetImguiMenus();

  /**
   * @brief Cluster function to make cleaner code while calling the rest of the imgui functions
   * 
   *@param e Engine reference to take the info from
   */
  static void ImguiFunctionCalls(ComponentManager* cm, RenderSystem* rs, SceneManager* sm);
};

#endif //__IMGUI_FUNCTIONS_HPP__