#ifndef __SCENE_MANAGER_HPP__
#define __SCENE_MANAGER_HPP__ 1

#include <defines.hpp>
#include <memory>
#include <vector>


#include <filesystem>
#include <fstream>

struct sqlite3;

#include <component_system.hpp>
#ifdef RENDER_OPENGL
  #include <render_system_opengl.hpp>
#endif
#ifdef RENDER_DIRECTX11
  #include <render_system_directx11.hpp>
#endif 

/**
 * @brief Struct that holds the information needed to make a new scene
 */
struct SceneData {
  char alias[DATABASE_ALIAS_LENGTH];
  char path[DATABASE_ALIAS_LENGTH];
  std::filesystem::file_time_type last_update;

  SceneData() {
    alias[0] = '\0';
    path[0] = '\0';
  }
};

/**
 * @brief Class that manages the scene functions, such as loading, saving, and others
 */
class SceneManager {

public:
  SceneManager();
  ~SceneManager();

  /**
   * @brief Store all the contents of the actual scene into a new one with a given alias
   *
   * @param scene_alias Name to display into the new scene
   *
   * @return int Returns -1 if there is an error, if not return 0
   */
  int SaveNewScene(ComponentManager* component_manager, RenderSystem* render_system, std::string scene_alias);

  /**
  * @brief Load all contents of a previously saved scene
  *
  * @param scene_name Which scene to load
  *
  * @return bool If the database was succesfully loaded or not
  */
  bool LoadScene(ComponentManager* component_manager, RenderSystem* render_system, std::string scene_name);

  /**
   * @brief Retrieve all the databases in the database folder
   * 
   */
  void QueryScenes();

  /**
   * @brief Delete a selected scene from the database
   * 
   * @param scene The scene to be deleted
   */
  void DeleteScene(SceneData scene);

  /**
   * @brief Print last error of given database
   */
  static void PrintLastError(sqlite3* db);

  /**
   * @brief Returns true or false if the database exists already
   * 
   * @param db_name
   */
  static bool ExistsDB(std::string db_name);

  /**
   * @brief Deletes the given db file
   * 
   * @param db_name 
   */
  static bool DeleteDB(std::string db_name);

  /** List of loaded scenes from the db */
  std::vector<SceneData> scenes_;

  /** String of queries to create a database */
  std::string database_creation_queries_;
private:

  /**
   * @brief Preload the queries needed to form the database.
   * 
   */
  void PreLoadDBCreationQueries();

  /** If the database has loaded or not */
  bool loaded_db_;

  /** Id of the loaded scene */
  std::string scene_selected_;

};


#endif //__SCENE_MANAGER_HPP__