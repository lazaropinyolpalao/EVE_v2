#include "scene_manager.hpp"
#include <sqlite3.h>


SceneManager::SceneManager(){
  scene_selected_ = "";

  PreLoadDBCreationQueries();
  QueryScenes();

}
 
SceneManager::~SceneManager(){
  /*
  if (loaded_db_) {
    printf("\033[0;32m Database closed succesfully\n \033[0m");
    sqlite3_close(db);
  }
 */
}


int SceneManager::SaveNewScene(ComponentManager* component_manager, RenderSystem* render_system, std::string scene_alias) {

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();
  static size_t camera_hash = typeid(CameraComponent).hash_code();
  static size_t tree_hash = typeid(TreeComponent).hash_code();

  //If no given name, use today's date as name with a prefix
  if (scene_alias.empty()) {
    time_t buf = time(NULL);
    tm t;
    localtime_s(&t, &buf);

    scene_alias = "UnnamedDatabase-";
    char temp[128];
    sprintf_s(temp, "%d-%02d-%02d_%02d.%02d.%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_min);
    scene_alias.append(temp);
  }

  char str[1024];
  int step_result = 0;
  sqlite3* db = nullptr;
  sqlite3_stmt* prepared_stmt;

  //Create new database in the default location with the given or created alias
  std::string t = DATABASE_PATH + scene_alias;
  //Add ".db" ending
  t.append(".db");
  if (sqlite3_open(t.c_str(), &db) != SQLITE_OK) {
    printf("\x1b[31m Error creating database in path: %s \x1b[0m\n", t.c_str());
    SceneManager::PrintLastError(db);
    return -1;
  }


  //Create database tables
  sqlite3_exec(db, database_creation_queries_.c_str(), nullptr, nullptr, nullptr);

  //Allow foreign keys
  sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

  sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

  //Delete all the previous registers in case of an overwriting before writing any registers
  sqlite3_exec(db, "DELETE FROM textures;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM meshes;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM screen_text;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM directional_light;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM spot_light;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM point_light;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "DELETE FROM entities;", nullptr, nullptr, nullptr);

  //Save each texture src
  const unsigned int num_textures = (unsigned int)render_system->resource_list_.textures_.size();
  strcpy_s(str, "INSERT INTO textures (texture_id, texture_src) VALUES (?1, ?2);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < num_textures; ++i) {
    sqlite3_bind_int(prepared_stmt, 1, i);
    sqlite3_bind_text(prepared_stmt, 2, render_system->resource_list_.textures_.at(i)->src_.c_str(), -1, SQLITE_STATIC);

    step_result = sqlite3_step(prepared_stmt);
    if (step_result != SQLITE_DONE) {
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      /*SceneManager::PrintLastError(db);*/
      printf("Failed at texture %d\n", i);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);
  }

  //Save each mesh src
  const unsigned int num_meshes = (unsigned int)render_system->resource_list_.meshes_.size();
  strcpy_s(str, "INSERT INTO meshes (mesh_id, mesh_src, cull_type) VALUES (?1, ?2, ?3);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < num_meshes; ++i) {
    sqlite3_bind_int(prepared_stmt, 1, i);
    sqlite3_bind_text(prepared_stmt, 2, render_system->resource_list_.meshes_.at(i)->full_path_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(prepared_stmt, 3, render_system->resource_list_.meshes_.at(i)->cull_type_);

    step_result = sqlite3_step(prepared_stmt);
    if (step_result != SQLITE_DONE) {
      printf("Failed at mesh %d\n", i);
      /*SceneManager::PrintLastError(db);*/
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);
  }

  //Save each onscreen text
  const unsigned int num_texts = (unsigned int)render_system->resource_list_.screen_texts_.size();
  strcpy_s(str, "INSERT INTO screen_text (screen_text_id, text_, color_x, color_y, color_z, pos_x, pos_y, scale) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < num_texts; ++i) {
      RenderingText* r = render_system->resource_list_.screen_texts_.at(i).get();
      sqlite3_bind_int(prepared_stmt, 1, i);
      sqlite3_bind_text(prepared_stmt, 2, r->text_.c_str(), -1, SQLITE_STATIC);
      //Color xyz
      sqlite3_bind_double(prepared_stmt, 3, (double)r->color_.x);
      sqlite3_bind_double(prepared_stmt, 4, (double)r->color_.y);
      sqlite3_bind_double(prepared_stmt, 5, (double)r->color_.z);
      //Pos xy and scale
      sqlite3_bind_double(prepared_stmt, 6, (double)r->pos_x);
      sqlite3_bind_double(prepared_stmt, 7, (double)r->pos_y);
      sqlite3_bind_double(prepared_stmt, 8, (double)r->scale_);


      step_result = sqlite3_step(prepared_stmt);
      if (step_result != SQLITE_DONE) {
          printf("Failed at screen text %d\n", i);
          SceneManager::PrintLastError(db);
          sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
          SceneManager::DeleteDB(scene_alias);
          return -1;
      }
      sqlite3_reset(prepared_stmt);
  }


  //Save each entity
  strcpy_s(str, "INSERT INTO entities (entity_id) VALUES (?1);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  std::vector<size_t>::iterator s, e;
  s = component_manager->deleted_entities_.begin();
  e = component_manager->deleted_entities_.end();
  size_t size = component_manager->deleted_entities_.size();
  for (unsigned int i = 0; i < component_manager->num_entities_; ++i) {
    //Add it to the database only if not deleted
    if (size == 0 || std::find(s, e, i) != e) {
      sqlite3_bind_int(prepared_stmt, 1, i + 1);

      step_result = sqlite3_step(prepared_stmt);
      if (step_result != SQLITE_DONE) {
        printf("Failed at entity %d\n", i + 1);
        /*/
        SceneManager::PrintLastError(db);
        /**/
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        SceneManager::DeleteDB(scene_alias);
        return -1;
      }
      sqlite3_reset(prepared_stmt);
    }
  }

  //Save each directional light
  strcpy_s(str, "INSERT INTO directional_light (directional_id, visible, position_x, position_y, position_z, ambient_x, ambient_y, ambient_z, diffuse_x, diffuse_y, diffuse_z, specular_x, specular_y, specular_z, znear, zfar, left_, right_, bottom_, top_, yaw, pitch) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < render_system->lights_.directional_.size(); ++i) {
    DirectionalLight* d = render_system->lights_.directional_[i].get();

    //Bind each parameter before doing the step()
    if (d != nullptr) {
      sqlite3_bind_int(prepared_stmt, 1, i);
      sqlite3_bind_int(prepared_stmt, 2, d->visible_);
      //Position xyz
      sqlite3_bind_double(prepared_stmt, 3, (double)d->position_.x);
      sqlite3_bind_double(prepared_stmt, 4, (double)d->position_.y);
      sqlite3_bind_double(prepared_stmt, 5, (double)d->position_.z);
      //Ambient xyz
      sqlite3_bind_double(prepared_stmt, 6, (double)d->ambient_.x);
      sqlite3_bind_double(prepared_stmt, 7, (double)d->ambient_.y);
      sqlite3_bind_double(prepared_stmt, 8, (double)d->ambient_.z);
      //Diffuse xyz
      sqlite3_bind_double(prepared_stmt, 9, (double)d->diffuse_.x);
      sqlite3_bind_double(prepared_stmt, 10, (double)d->diffuse_.y);
      sqlite3_bind_double(prepared_stmt, 11, (double)d->diffuse_.z);
      //Specular xyz
      sqlite3_bind_double(prepared_stmt, 12, (double)d->specular_.x);
      sqlite3_bind_double(prepared_stmt, 13, (double)d->specular_.y);
      sqlite3_bind_double(prepared_stmt, 14, (double)d->specular_.z);
      //Near and far
      sqlite3_bind_double(prepared_stmt, 15, (double)d->znear_);
      sqlite3_bind_double(prepared_stmt, 16, (double)d->zfar_);
      //left, right, bottom, top
      sqlite3_bind_double(prepared_stmt, 17, (double)d->znear_);
      sqlite3_bind_double(prepared_stmt, 18, (double)d->left_);
      sqlite3_bind_double(prepared_stmt, 19, (double)d->right_);
      sqlite3_bind_double(prepared_stmt, 20, (double)d->bottom_);
      //Yaw, pitch
      sqlite3_bind_double(prepared_stmt, 21, (double)d->yaw_);
      sqlite3_bind_double(prepared_stmt, 22, (double)d->pitch_);

      step_result = sqlite3_step(prepared_stmt);
      if (step_result != SQLITE_DONE) {
        printf("Failed at directional light %d\n", i);
        /*/
        SceneManager::PrintLastError(db);
        /**/
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        SceneManager::DeleteDB(scene_alias);
        return -1;
      }
      sqlite3_reset(prepared_stmt);
    }

  }

  //Save each spot light
  strcpy_s(str, "INSERT INTO spot_light (spot_id, visible, position_x, position_y, position_z, ambient_x, ambient_y, ambient_z, diffuse_x, diffuse_y, diffuse_z, specular_x, specular_y, specular_z, znear, zfar, aspect_ratio, fov, yaw, pitch, cut_off, outer_cut_off, constant, linear, quadratic, range) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22, ?23, ?24, ?25, ?26);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < render_system->lights_.spot_.size(); ++i) {
    SpotLight* s = render_system->lights_.spot_[i].get();

    //Bind each parameter before doing the step()
    if (s != nullptr) {
      sqlite3_bind_int(prepared_stmt, 1, i);
      sqlite3_bind_int(prepared_stmt, 2, s->visible_);
      //Position xyz
      sqlite3_bind_double(prepared_stmt, 3, (double)s->position_.x);
      sqlite3_bind_double(prepared_stmt, 4, (double)s->position_.y);
      sqlite3_bind_double(prepared_stmt, 5, (double)s->position_.z);
      //Ambient xyz
      sqlite3_bind_double(prepared_stmt, 6, (double)s->ambient_.x);
      sqlite3_bind_double(prepared_stmt, 7, (double)s->ambient_.y);
      sqlite3_bind_double(prepared_stmt, 8, (double)s->ambient_.z);
      //Diffuse xyz
      sqlite3_bind_double(prepared_stmt, 9, (double)s->diffuse_.x);
      sqlite3_bind_double(prepared_stmt, 10, (double)s->diffuse_.y);
      sqlite3_bind_double(prepared_stmt, 11, (double)s->diffuse_.z);
      //Specular xyz
      sqlite3_bind_double(prepared_stmt, 12, (double)s->specular_.x);
      sqlite3_bind_double(prepared_stmt, 13, (double)s->specular_.y);
      sqlite3_bind_double(prepared_stmt, 14, (double)s->specular_.z);
      //Near and far
      sqlite3_bind_double(prepared_stmt, 15, (double)s->znear_);
      sqlite3_bind_double(prepared_stmt, 16, (double)s->zfar_);
      //Aspect ratio, fov
      sqlite3_bind_double(prepared_stmt, 17, (double)s->aspect_ratio_);
      sqlite3_bind_double(prepared_stmt, 18, (double)s->fov_);
      //Yaw, pitch
      sqlite3_bind_double(prepared_stmt, 19, (double)s->yaw_);
      sqlite3_bind_double(prepared_stmt, 20, (double)s->pitch_);
      //Cutoff, outer cutoff, constant, 
      sqlite3_bind_double(prepared_stmt, 21, (double)s->cut_off_);
      sqlite3_bind_double(prepared_stmt, 22, (double)s->outer_cut_off_);
      sqlite3_bind_double(prepared_stmt, 23, (double)s->constant_);
      //linear, qudratic, range
      sqlite3_bind_double(prepared_stmt, 24, (double)s->linear_);
      sqlite3_bind_double(prepared_stmt, 25, (double)s->quadratic_);
      sqlite3_bind_double(prepared_stmt, 26, (double)s->range_);

      step_result = sqlite3_step(prepared_stmt);
      if (step_result != SQLITE_DONE) {
        printf("Failed at spot light %d\n", i);
        //*/
        SceneManager::PrintLastError(db);
        /**/
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        SceneManager::DeleteDB(scene_alias);
        return -1;
      }
      sqlite3_reset(prepared_stmt);
    }

  }

  //Save each point light
  strcpy_s(str, "INSERT INTO point_light (point_id, visible, position_x, position_y, position_z, ambient_x, ambient_y, ambient_z, diffuse_x, diffuse_y, diffuse_z, specular_x, specular_y, specular_z, znear, zfar, aspect_ratio, constant, linear, quadratic, range) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (unsigned int i = 0; i < render_system->lights_.point_.size(); ++i) {
    PointLight* p = render_system->lights_.point_[i].get();

    //Bind each parameter before doing the step()
    if (p != nullptr) {
      sqlite3_bind_int(prepared_stmt, 1, i);
      sqlite3_bind_int(prepared_stmt, 2, p->visible_);
      //Position xyz
      sqlite3_bind_double(prepared_stmt, 3, (double)p->position_.x);
      sqlite3_bind_double(prepared_stmt, 4, (double)p->position_.y);
      sqlite3_bind_double(prepared_stmt, 5, (double)p->position_.z);
      //Ambient xyz
      sqlite3_bind_double(prepared_stmt, 6, (double)p->ambient_.x);
      sqlite3_bind_double(prepared_stmt, 7, (double)p->ambient_.y);
      sqlite3_bind_double(prepared_stmt, 8, (double)p->ambient_.z);
      //Diffuse xyz
      sqlite3_bind_double(prepared_stmt, 9, (double)p->diffuse_.x);
      sqlite3_bind_double(prepared_stmt, 10, (double)p->diffuse_.y);
      sqlite3_bind_double(prepared_stmt, 11, (double)p->diffuse_.z);
      //Specular xyz
      sqlite3_bind_double(prepared_stmt, 12, (double)p->specular_.x);
      sqlite3_bind_double(prepared_stmt, 13, (double)p->specular_.y);
      sqlite3_bind_double(prepared_stmt, 14, (double)p->specular_.z);
      //Near and far
      sqlite3_bind_double(prepared_stmt, 15, (double)p->znear_);
      sqlite3_bind_double(prepared_stmt, 16, (double)p->zfar_);
      //Aspect ratio, 
      sqlite3_bind_double(prepared_stmt, 17, (double)p->aspect_ratio_);
      //constant, linear, qudratic, range
      sqlite3_bind_double(prepared_stmt, 18, (double)p->constant_);
      sqlite3_bind_double(prepared_stmt, 19, (double)p->linear_);
      sqlite3_bind_double(prepared_stmt, 20, (double)p->quadratic_);
      sqlite3_bind_double(prepared_stmt, 21, (double)p->range_);

      step_result = sqlite3_step(prepared_stmt);
      if (step_result != SQLITE_DONE) {
        printf("Failed at point light %d\n", i);
        /*/
        SceneManager::PrintLastError(db);
        /**/
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        SceneManager::DeleteDB(scene_alias);
        return -1;
      }
      sqlite3_reset(prepared_stmt);
    }

  }

  //Save each transform
  std::vector<component_node<TransformComponent>>* transform_components = &(*static_cast<component_list<TransformComponent>*>(component_manager->components_classes_.find(transf_hash)->second.get())).components_;
  size_t transf_size = transform_components->size();
  strcpy_s(str, "INSERT INTO transform (transform_id, entity_id, position_x, position_y, position_z, rotation_x, rotation_y, rotation_z, scale_x, scale_y, scale_z) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (size_t i = 0; i < transf_size; ++i) {
    TransformComponent* t = &(transform_components->at(i).data_);

    sqlite3_bind_int(prepared_stmt, 1, (int)i);
    sqlite3_bind_int(prepared_stmt, 2, (int)transform_components->at(i).entity_id_);
    //Position xyz
    glm::vec3 position = t->GetPosition(), rotation = t->GetRotation(), scale = t->GetScale();
    sqlite3_bind_double(prepared_stmt, 3, (double)position.x);
    sqlite3_bind_double(prepared_stmt, 4, (double)position.y);
    sqlite3_bind_double(prepared_stmt, 5, (double)position.z);
    //Rotation xyz
    sqlite3_bind_double(prepared_stmt, 6, (double)rotation.x);
    sqlite3_bind_double(prepared_stmt, 7, (double)rotation.y);
    sqlite3_bind_double(prepared_stmt, 8, (double)rotation.z);
    //Scale xyz
    sqlite3_bind_double(prepared_stmt, 9, (double)scale.x);
    sqlite3_bind_double(prepared_stmt, 10, (double)scale.y);
    sqlite3_bind_double(prepared_stmt, 11, (double)scale.z);


    step_result = sqlite3_step(prepared_stmt);
    if (step_result != SQLITE_DONE) {
      printf("Failed at transform %zd\n", i);
      /*/
      SceneManager::PrintLastError(db);
      /**/
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);
  }

  //Save each renderer and it's associated textures and mesh
  std::vector<component_node<RendererComponent>>* renderer_components = &(*static_cast<component_list<RendererComponent>*>(component_manager->components_classes_.find(render_hash)->second.get())).components_;
  size_t render_size = renderer_components->size();
  for (size_t i = 0; i < render_size; ++i) {
    strcpy_s(str, "INSERT INTO renderer (render_id, entity_id, needs_light, casts_shadow, receives_shadows) VALUES (?1, ?2, ?3, ?4, ?5);");
    sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
    RendererComponent* r = &(renderer_components->at(i).data_);

    sqlite3_bind_int(prepared_stmt, 1, (int)i);
    sqlite3_bind_int(prepared_stmt, 2, (int)renderer_components->at(i).entity_id_);
    //Light, cast, receives
    sqlite3_bind_int(prepared_stmt, 3, r->needs_light_);
    sqlite3_bind_int(prepared_stmt, 4, r->casts_shadows_);
    sqlite3_bind_int(prepared_stmt, 5, r->receives_shadows_);

    step_result = sqlite3_step(prepared_stmt);
    if (step_result != SQLITE_DONE) {
      printf("Failed at renderer %zd\n", i);
      /*/
      SceneManager::PrintLastError(db);
      /**/
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);

    //Save each texture of each renderer
    if (r->textures_.size() != 0) {
      strcpy_s(str, "INSERT INTO textures_of_renderer (render_id, texture_id) VALUES (?1, ?2);");
      sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
      //Find the textures and add them in the order that the render has them
      for (size_t j = 0; j < r->textures_.size(); j++) {

        std::shared_ptr<Texture> texture = r->textures_.at(j);
        size_t pos = -1;
        for (size_t k = 0; pos == -1 && k < num_textures; ++k) {
          //If same address, it's the same texture
          if (texture == render_system->resource_list_.textures_.at(k)) {
            pos = k;
          }
        }

        if (pos != -1) {
          sqlite3_bind_int(prepared_stmt, 1, (int)i);
          sqlite3_bind_int(prepared_stmt, 2, (int)pos);

          step_result = sqlite3_step(prepared_stmt);
          if (step_result != SQLITE_DONE) {
            printf("Failed at texture %zd from renderer %zd \n", j, i);
            /*/
            SceneManager::PrintLastError(db);
            /**/
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            SceneManager::DeleteDB(scene_alias);
            return -1;
          }
        }
        sqlite3_reset(prepared_stmt);
      }
    }

    //Save associated mesh if loaded
    if (r->isInit_ && r->mesh_ != nullptr && r->mesh_->isInit_) {
      strcpy_s(str, "INSERT INTO meshes_of_renderer (render_id, mesh_id) VALUES (?1, ?2);");
      sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
      std::shared_ptr<TinyObj> mesh = r->mesh_;

      size_t pos = -1;
      for (size_t j = 0; pos == -1 && j < num_meshes; ++j) {
        //If same address, it's the same texture
        if (mesh == render_system->resource_list_.meshes_.at(j)) {
          pos = j;
        }
      }

      if (pos != -1) {
        sqlite3_bind_int(prepared_stmt, 1, (int)i);
        sqlite3_bind_int(prepared_stmt, 2, (int)pos);

        step_result = sqlite3_step(prepared_stmt);
        if (step_result != SQLITE_DONE) {
          printf("Failed at the mesh from renderer %zd \n", i);
          /*/
          SceneManager::PrintLastError(db);
          /**/
          sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
          SceneManager::DeleteDB(scene_alias);
          return -1;
        }
      }
      sqlite3_reset(prepared_stmt);
    }

  }

  //Save each camera
  std::vector<component_node<CameraComponent>>* camera_components = &(*static_cast<component_list<CameraComponent>*>(component_manager->components_classes_.find(camera_hash)->second.get())).components_;
  size_t camera_size = camera_components->size();
  strcpy_s(str, "INSERT INTO camera (camera_id, entity_id, is_active, mode, pitch, yaw, roll, position_x, position_y, position_z, znear, zfar, left_, right_, bottom_, top_, fov, aspect_ratio) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (size_t i = 0; i < camera_size; ++i) {
    CameraComponent* c = &(camera_components->at(i).data_);

    sqlite3_bind_int(prepared_stmt, 1, (int)i);
    sqlite3_bind_int(prepared_stmt, 2, (int)camera_components->at(i).entity_id_);



    //Active, Mode, pitch, yaw, roll
    sqlite3_bind_int(prepared_stmt, 3, (int)c->is_active_);
    sqlite3_bind_int(prepared_stmt, 4, static_cast<int>(c->get_mode()));
    sqlite3_bind_double(prepared_stmt, 5, c->get_pitch());
    sqlite3_bind_double(prepared_stmt, 6, c->get_yaw());
    sqlite3_bind_double(prepared_stmt, 7, c->get_roll());
    //Position
    sqlite3_bind_double(prepared_stmt, 8, c->get_position().x);
    sqlite3_bind_double(prepared_stmt, 9, c->get_position().y);
    sqlite3_bind_double(prepared_stmt, 10, c->get_position().z);

    //znear, far
    sqlite3_bind_double(prepared_stmt, 11, c->get_near());
    sqlite3_bind_double(prepared_stmt, 12, c->get_far());

    //left, right, bottom, top
    sqlite3_bind_double(prepared_stmt, 13, c->get_orthographic_left());
    sqlite3_bind_double(prepared_stmt, 14, c->get_orthographic_right());
    sqlite3_bind_double(prepared_stmt, 15, c->get_orthographic_bottom());
    sqlite3_bind_double(prepared_stmt, 16, c->get_orthographic_top());

    //fov, aspect_ratio
    sqlite3_bind_double(prepared_stmt, 17, c->get_fov());
    sqlite3_bind_double(prepared_stmt, 18, c->get_aspect_ratio());

    step_result = sqlite3_step(prepared_stmt);
    if (step_result != SQLITE_DONE) {
      printf("Failed at camera %zd\n", i);
      /*SceneManager::PrintLastError(db);*/
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);
  }


  //Save each tree
  std::vector<component_node<TreeComponent>>* tree_comps = &(*static_cast<component_list<TreeComponent>*>(component_manager->components_classes_.find(tree_hash)->second.get())).components_;
  size_t tree_size = tree_comps->size();
  strcpy_s(str, "INSERT INTO tree (tree_id, entity_id, parent_id, name) VALUES (?1, ?2, ?3, ?4);");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  for (size_t i = 0; i < tree_size; ++i) {
    TreeComponent* t = &(tree_comps->at(i).data_);

    sqlite3_bind_int(prepared_stmt, 1, (int)i);
    sqlite3_bind_int(prepared_stmt, 2, (int)tree_comps->at(i).entity_id_);
    sqlite3_bind_int(prepared_stmt, 3, (int)t->parent_);
    sqlite3_bind_text(prepared_stmt, 4, t->name, -1, SQLITE_STATIC);

    if (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
      printf("Failed at tree component %zd\n", i);
      SceneManager::PrintLastError(db);
      sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
      SceneManager::DeleteDB(scene_alias);
      return -1;
    }
    sqlite3_reset(prepared_stmt);

  }

  //Commit changes if no errors and reload the scenes
  sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
  printf("Database saved succesfully");

  sqlite3_finalize(prepared_stmt);
  sqlite3_close(db);

  QueryScenes();

  return 0;
}

bool SceneManager::LoadScene(ComponentManager* component_manager, RenderSystem* render_system, std::string scene_name) {

  static size_t render_hash = typeid(RendererComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();
  static size_t camera_hash = typeid(CameraComponent).hash_code();
  static size_t tree_hash = typeid(TreeComponent).hash_code();

  //If empty or doesn't exist, exit
  if (scene_name.empty() || !SceneManager::ExistsDB(scene_name)) { 
      printf("missing scene %s\n", scene_name.c_str());
      return false; 
  }

  char str[1024];
  int step_result = 0;
  sqlite3* db = nullptr;
  sqlite3_stmt* prepared_stmt;
  std::string fullpath = DATABASE_PATH + scene_name;
  fullpath.append(".db");
  //Open connection
  if (sqlite3_open(fullpath.c_str(), &db) != SQLITE_OK) {
    printf("\x1b[31m Error opening database in path: %s \x1b[0m\n", scene_name.c_str());
    SceneManager::PrintLastError(db);
    return false;
  }

  //Reset current scene
  component_manager->ResetComponentSystem();

  //Load each texture
  strcpy_s(str, "SELECT texture_id, texture_src FROM textures;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    int id = sqlite3_column_int(prepared_stmt, 0);
    const char* t_src = (const char*)sqlite3_column_text(prepared_stmt, 1);
    #ifdef RENDER_OPENGL
      render_system->resource_list_.addTexture(t_src);
    #endif
    #ifdef RENDER_DIRECTX11
      render_system->resource_list_.addTexture(static_cast<RenderSystemDirectX11*>(render_system)->getDevice(), t_src);
    #endif 
  }
  sqlite3_reset(prepared_stmt);

  //Load each mesh
  strcpy_s(str, "SELECT mesh_id, mesh_src, cull_type FROM meshes;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    int id = sqlite3_column_int(prepared_stmt, 0);
    const char* t_src = (const char*)sqlite3_column_text(prepared_stmt, 1);
    int cull = sqlite3_column_int(prepared_stmt, 2);
    std::shared_ptr<TinyObj> shared;
    #ifdef RENDER_OPENGL
        shared = render_system->resource_list_.addMesh(t_src);
    #endif
    #ifdef RENDER_DIRECTX11
        RenderSystemDirectX11* r = static_cast<RenderSystemDirectX11*>(render_system);
        shared = render_system->resource_list_.addMesh(r->getDevice(), r->getDeviceContext(), t_src);
    #endif 
    shared->cull_type_ = cull;
  }
  sqlite3_reset(prepared_stmt);

  //Save each rendered text
  strcpy_s(str, "SELECT screen_text_id, text_, color_x, color_y, color_z, pos_x, pos_y, scale FROM screen_text;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
      int id = sqlite3_column_int(prepared_stmt, 0);
      const char* t_src = (const char*)sqlite3_column_text(prepared_stmt, 1);
      
      glm::vec3 color;
      color.x = (float)sqlite3_column_double(prepared_stmt, 2);
      color.y = (float)sqlite3_column_double(prepared_stmt, 3);
      color.z = (float)sqlite3_column_double(prepared_stmt, 4);
      float pos_x = (float)sqlite3_column_double(prepared_stmt, 5);
      float pos_y = (float)sqlite3_column_double(prepared_stmt, 6);
      float scale = (float)sqlite3_column_double(prepared_stmt, 7);
      
      std::string text = t_src;

#ifdef RENDER_OPENGL
      RenderSystemOpenGL* r = static_cast<RenderSystemOpenGL*>(render_system);
      
      r->resource_list_.addTextToRender(text, pos_x, pos_y, color, scale);
#endif
  }
  sqlite3_reset(prepared_stmt);


  //Load each light into the resources
  strcpy_s(str, "SELECT * FROM directional_light;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {

    int id = sqlite3_column_int(prepared_stmt, 0);
    bool visible = sqlite3_column_int(prepared_stmt, 1);
    float pos_x = (float)sqlite3_column_double(prepared_stmt, 2);
    float pos_y = (float)sqlite3_column_double(prepared_stmt, 3);
    float pos_z = (float)sqlite3_column_double(prepared_stmt, 4);

    float amb_x = (float)sqlite3_column_double(prepared_stmt, 5);
    float amb_y = (float)sqlite3_column_double(prepared_stmt, 6);
    float amb_z = (float)sqlite3_column_double(prepared_stmt, 7);

    float diff_x = (float)sqlite3_column_double(prepared_stmt, 8);
    float diff_y = (float)sqlite3_column_double(prepared_stmt, 9);
    float diff_z = (float)sqlite3_column_double(prepared_stmt, 10);

    float spec_x = (float)sqlite3_column_double(prepared_stmt, 11);
    float spec_y = (float)sqlite3_column_double(prepared_stmt, 12);
    float spec_z = (float)sqlite3_column_double(prepared_stmt, 13);

    float znear = (float)sqlite3_column_double(prepared_stmt, 14);
    float zfar = (float)sqlite3_column_double(prepared_stmt, 15);

    float left = (float)sqlite3_column_double(prepared_stmt, 16);
    float right = (float)sqlite3_column_double(prepared_stmt, 17);
    float bottom = (float)sqlite3_column_double(prepared_stmt, 18);
    float top = (float)sqlite3_column_double(prepared_stmt, 19);

    float yaw = (float)sqlite3_column_double(prepared_stmt, 20);
    float pitch = (float)sqlite3_column_double(prepared_stmt, 21);

    render_system->lights_.addDirectionalLight(
      glm::vec3(amb_x, amb_y, amb_z), glm::vec3(diff_x, diff_y, diff_z), glm::vec3(spec_x, spec_y, spec_z),
      pitch, yaw
    );

    DirectionalLight* d = render_system->lights_.directional_.back().get();
    d->visible_ = visible;
    d->position_ = glm::vec3(pos_x, pos_y, pos_z);
    d->znear_ = znear;
    d->zfar_ = zfar;
    d->left_ = left;
    d->right_ = right;
    d->bottom_ = bottom;
    d->top_ = top;
  }
  sqlite3_reset(prepared_stmt);

  strcpy_s(str, "SELECT * FROM spot_light;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {

    int id = sqlite3_column_int(prepared_stmt, 0);
    bool visible = sqlite3_column_int(prepared_stmt, 1);
    float pos_x = (float)sqlite3_column_double(prepared_stmt, 2);
    float pos_y = (float)sqlite3_column_double(prepared_stmt, 3);
    float pos_z = (float)sqlite3_column_double(prepared_stmt, 4);

    float amb_x = (float)sqlite3_column_double(prepared_stmt, 5);
    float amb_y = (float)sqlite3_column_double(prepared_stmt, 6);
    float amb_z = (float)sqlite3_column_double(prepared_stmt, 7);

    float diff_x = (float)sqlite3_column_double(prepared_stmt, 8);
    float diff_y = (float)sqlite3_column_double(prepared_stmt, 9);
    float diff_z = (float)sqlite3_column_double(prepared_stmt, 10);

    float spec_x = (float)sqlite3_column_double(prepared_stmt, 11);
    float spec_y = (float)sqlite3_column_double(prepared_stmt, 12);
    float spec_z = (float)sqlite3_column_double(prepared_stmt, 13);

    float znear = (float)sqlite3_column_double(prepared_stmt, 14);
    float zfar = (float)sqlite3_column_double(prepared_stmt, 15);

    float aspect_ratio = (float)sqlite3_column_double(prepared_stmt, 16);
    float fov = (float)sqlite3_column_double(prepared_stmt, 17);

    float yaw = (float)sqlite3_column_double(prepared_stmt, 18);
    float pitch = (float)sqlite3_column_double(prepared_stmt, 19);

    float cut_off = (float)sqlite3_column_double(prepared_stmt, 20);
    float outer_cut_off = (float)sqlite3_column_double(prepared_stmt, 21);

    float constant = (float)sqlite3_column_double(prepared_stmt, 22);
    float linear = (float)sqlite3_column_double(prepared_stmt, 23);
    float quadratic = (float)sqlite3_column_double(prepared_stmt, 24);
    float range = (float)sqlite3_column_double(prepared_stmt, 25);


    render_system->lights_.addSpotLight(
      glm::vec3(pos_x, pos_y, pos_z), glm::vec3(amb_x, amb_y, amb_z),
      glm::vec3(diff_x, diff_y, diff_z), glm::vec3(spec_x, spec_y, spec_z),
      cut_off, outer_cut_off, constant, linear, quadratic,
      pitch, yaw
    );

    SpotLight* s = render_system->lights_.spot_.back().get();
    s->visible_ = visible;
    s->aspect_ratio_ = aspect_ratio;
    s->fov_ = fov;
    s->range_ = range;

  }
  sqlite3_reset(prepared_stmt);

  strcpy_s(str, "SELECT * FROM point_light;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {

    int id = sqlite3_column_int(prepared_stmt, 0);
    bool visible = sqlite3_column_int(prepared_stmt, 1);
    float pos_x = (float)sqlite3_column_double(prepared_stmt, 2);
    float pos_y = (float)sqlite3_column_double(prepared_stmt, 3);
    float pos_z = (float)sqlite3_column_double(prepared_stmt, 4);

    float amb_x = (float)sqlite3_column_double(prepared_stmt, 5);
    float amb_y = (float)sqlite3_column_double(prepared_stmt, 6);
    float amb_z = (float)sqlite3_column_double(prepared_stmt, 7);

    float diff_x = (float)sqlite3_column_double(prepared_stmt, 8);
    float diff_y = (float)sqlite3_column_double(prepared_stmt, 9);
    float diff_z = (float)sqlite3_column_double(prepared_stmt, 10);

    float spec_x = (float)sqlite3_column_double(prepared_stmt, 11);
    float spec_y = (float)sqlite3_column_double(prepared_stmt, 12);
    float spec_z = (float)sqlite3_column_double(prepared_stmt, 13);

    float znear = (float)sqlite3_column_double(prepared_stmt, 14);
    float zfar = (float)sqlite3_column_double(prepared_stmt, 15);

    float aspect_ratio = (float)sqlite3_column_double(prepared_stmt, 16);

    float constant = (float)sqlite3_column_double(prepared_stmt, 17);
    float linear = (float)sqlite3_column_double(prepared_stmt, 18);
    float quadratic = (float)sqlite3_column_double(prepared_stmt, 19);
    float range = (float)sqlite3_column_double(prepared_stmt, 20);


    render_system->lights_.addPointLight(
      glm::vec3(pos_x, pos_y, pos_z), glm::vec3(amb_x, amb_y, amb_z),
      glm::vec3(diff_x, diff_y, diff_z), glm::vec3(spec_x, spec_y, spec_z),
      constant, linear, quadratic
    );

    PointLight* p = render_system->lights_.point_.back().get();
    p->visible_ = visible;
    p->aspect_ratio_ = aspect_ratio;
    p->range_ = range;

  }
  sqlite3_reset(prepared_stmt);


  //Create each entity and map it
  
  //This is needed since the entity saved as "10" in the DB can be now the entity "9" because there are not deleted entities saved
  std::map<size_t, size_t> entity_correspondance;
  strcpy_s(str, "SELECT * FROM entities;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    size_t id = (size_t)sqlite3_column_int(prepared_stmt, 0);
    entity_correspondance[id] = component_manager->new_entity();
  }
  sqlite3_reset(prepared_stmt);

  //Create each transform component
  strcpy_s(str, "SELECT * FROM transform;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    size_t id = (size_t)sqlite3_column_int(prepared_stmt, 0);

    size_t entity_id = (size_t)sqlite3_column_int(prepared_stmt, 1);

    float pos_x = (float)sqlite3_column_double(prepared_stmt, 2);
    float pos_y = (float)sqlite3_column_double(prepared_stmt, 3);
    float pos_z = (float)sqlite3_column_double(prepared_stmt, 4);

    float rot_x = (float)sqlite3_column_double(prepared_stmt, 5);
    float rot_y = (float)sqlite3_column_double(prepared_stmt, 6);
    float rot_z = (float)sqlite3_column_double(prepared_stmt, 7);

    float scale_x = (float)sqlite3_column_double(prepared_stmt, 8);
    float scale_y = (float)sqlite3_column_double(prepared_stmt, 9);
    float scale_z = (float)sqlite3_column_double(prepared_stmt, 10);

    //Create the component for the mapped entity
    TransformComponent* t = component_manager->addComponent<TransformComponent>(entity_correspondance[entity_id]);
    t->SetTranslation(pos_x, pos_y, pos_z);
    t->SetScale(scale_x, scale_y, scale_z);
    t->AddRotation(rot_x, rot_y, rot_z);
  }
  sqlite3_reset(prepared_stmt);

  //Get the number of renderers in the table, this is needed to be able to link each mesh and texture to it's renderer
  strcpy_s(str, "SELECT COUNT (*) FROM renderer;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  sqlite3_step(prepared_stmt);
  size_t num_renderers = (size_t)sqlite3_column_int(prepared_stmt, 0);
  sqlite3_reset(prepared_stmt);

  //Create each render component and it's associated textures and mesh
  for (size_t i = 0; i < num_renderers; ++i) {
    strcpy_s(str, "SELECT entity_id, needs_light, casts_shadow, receives_shadows FROM renderer WHERE render_id = ?1;");
    sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
    sqlite3_bind_int(prepared_stmt, 1, (int)i);

    if (sqlite3_step(prepared_stmt) != SQLITE_OK) {

      size_t entity_id = sqlite3_column_int(prepared_stmt, 0);
      bool needs_light = sqlite3_column_int(prepared_stmt, 1);
      bool casts_shadow = sqlite3_column_int(prepared_stmt, 2);
      bool receives_shadows = sqlite3_column_int(prepared_stmt, 3);
      sqlite3_reset(prepared_stmt);

      RendererComponent* r = component_manager->addComponent<RendererComponent>(entity_correspondance[entity_id]);
      r->needs_light_ = needs_light;
      r->casts_shadows_ = casts_shadow;
      r->receives_shadows_ = receives_shadows;

      //Load the textures
      strcpy_s(str, "SELECT texture_id FROM textures_of_renderer WHERE render_id = ?1");
      sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
      sqlite3_bind_int(prepared_stmt, 1, (int)i);
      while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
        size_t text_pos = sqlite3_column_int(prepared_stmt, 0);
        r->AddTexture(render_system->resource_list_.textures_.at(text_pos));
      }
      sqlite3_reset(prepared_stmt);

      //Load mesh
      strcpy_s(str, "SELECT mesh_id FROM meshes_of_renderer WHERE render_id = ?1");
      sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
      sqlite3_bind_int(prepared_stmt, 1, (int)i);
      while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
        size_t mesh_id = sqlite3_column_int(prepared_stmt, 0);
        r->Init(render_system->resource_list_.meshes_.at(mesh_id));
      }
      sqlite3_reset(prepared_stmt);

    }
  }

  //Camera components
  strcpy_s(str, "SELECT * FROM camera;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    size_t camera_id = (size_t)sqlite3_column_int(prepared_stmt, 0);
    size_t entity_id = (size_t)sqlite3_column_int(prepared_stmt, 1);
    bool is_active_ = sqlite3_column_int(prepared_stmt, 2);
    CameraMode mode = (CameraMode)sqlite3_column_int(prepared_stmt, 3);

    float pitch = (float)sqlite3_column_double(prepared_stmt, 4);
    float yaw = (float)sqlite3_column_double(prepared_stmt, 5);
    float roll = (float)sqlite3_column_double(prepared_stmt, 6);
    float pos_x = (float)sqlite3_column_double(prepared_stmt, 7);
    float pos_y = (float)sqlite3_column_double(prepared_stmt, 8);
    float pos_z = (float)sqlite3_column_double(prepared_stmt, 9);
    float znear = (float)sqlite3_column_double(prepared_stmt, 10);
    float zfar = (float)sqlite3_column_double(prepared_stmt, 11);
    float left = (float)sqlite3_column_double(prepared_stmt, 12);
    float right = (float)sqlite3_column_double(prepared_stmt, 13);
    float bottom = (float)sqlite3_column_double(prepared_stmt, 14);
    float top = (float)sqlite3_column_double(prepared_stmt, 15);
    float fov = (float)sqlite3_column_double(prepared_stmt, 16);
    float aspect_ratio = (float)sqlite3_column_double(prepared_stmt, 17);

    CameraComponent* c = component_manager->addComponent<CameraComponent>(entity_correspondance[entity_id]);
    c->is_active_ = is_active_;
    c->set_mode(mode);
    c->set_pitch(pitch);
    c->set_yaw(yaw);
    c->set_roll(roll);
    c->set_position(pos_x, pos_y, pos_z);
    c->set_near(znear);
    c->set_far(zfar);
    c->set_orthographic_limits(left, right, bottom, top);
    c->set_fov(fov);
    c->set_aspect_ratio(aspect_ratio);

    //Force update
    if (mode == CameraMode::kOrthographic) {
      c->SetOrthographic(left, right, bottom, top, znear, zfar);
    }
    else if (mode == CameraMode::kPerspective) {
      c->SetPerspective(fov, aspect_ratio, znear, zfar);
    }
    c->UpdateCamera({}, 0.0f);

  }
  sqlite3_reset(prepared_stmt);


  //Set the tree components with its saved name
  strcpy_s(str, "SELECT * FROM tree;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
      size_t tree_id = (size_t)sqlite3_column_int(prepared_stmt, 0);
      size_t entity_id = (size_t)sqlite3_column_int(prepared_stmt, 1);
      size_t parent_id = (size_t)sqlite3_column_int(prepared_stmt, 2);
      const char* t_src = (const char*)sqlite3_column_text(prepared_stmt, 3);

      TreeComponent* t = component_manager->get_component<TreeComponent>(entity_correspondance[entity_id]);
      if (strlen(t_src) == 0) {
          strcpy_s(t->name, "Unknown");
      }
      else {
        strcpy_s(t->name, t_src);
      }
  }
  sqlite3_reset(prepared_stmt);

  //Recreate the parenting, only of those who have parent
  //THIS HAS TO BE MADE THE LAST COMPONENT ALWAYS SINCE THE PARENTING SWAPS COMPONENTS AND ENTITIES
  strcpy_s(str, "SELECT * FROM tree WHERE parent_id != 0;");
  sqlite3_prepare_v2(db, str, -1, &prepared_stmt, NULL);
  while (sqlite3_step(prepared_stmt) != SQLITE_DONE) {
    size_t tree_id = (size_t)sqlite3_column_int(prepared_stmt, 0);
    size_t entity_id = (size_t)sqlite3_column_int(prepared_stmt, 1);
    size_t parent_id = (size_t)sqlite3_column_int(prepared_stmt, 2);
    const char* t_src = (const char*)sqlite3_column_text(prepared_stmt, 4);
    

    component_manager->make_parent(entity_correspondance[parent_id], entity_correspondance[entity_id]);
  }
  sqlite3_reset(prepared_stmt);

  return true;
}


void SceneManager::QueryScenes(){
  
  scenes_.clear();
#ifdef RENDER_OPENGL
  for (auto const& entry : std::filesystem::directory_iterator{ DATABASE_PATH }) {
    std::string path = entry.path().string();
    std::string str = path.substr(path.rfind("."));

    //Save only if the file ends with '.db'
    if (str.compare(".db")==0) {
      std::filesystem::file_time_type edit_time = std::filesystem::last_write_time(entry.path());

      SceneData sc;
      size_t pos = path.rfind("/") + 1;
      strcpy_s(sc.alias, path.substr(pos, path.length() - (pos-1)).c_str());
      strcpy_s(sc.path, path.c_str());
      sc.last_update = edit_time;

      scenes_.push_back(sc);
    }
  }
#endif
}

void SceneManager::DeleteScene(SceneData scene){
  //if (IsDBLoaded()) {
    //char str[256];
    //sprintf_s(str, "DELETE FROM scenes WHERE id = '%d';", scene_id);

    //sqlite3_exec(db, str, nullptr, this, nullptr);
    std::remove(scene.path);
    QueryScenes();
  //}
}

void SceneManager::PreLoadDBCreationQueries(){
  //Open file and load it's whole content into the string
  std::ifstream ifs(DATABASE_CREATION_SQL_PATH);
  std::string content((std::istreambuf_iterator<char>(ifs)),(std::istreambuf_iterator<char>()));
  database_creation_queries_ = content;
  //printf("%s\n", database_creation_queries_.c_str());
}

void SceneManager::PrintLastError(sqlite3* db){
  if (db != nullptr) {
    printf("Error: %s\n", sqlite3_errmsg(db));
  }
}

bool SceneManager::ExistsDB(std::string db_name){

  //Add the ".db" characters to the db_name for searches
  std::string full_db = db_name + ".db";
#ifdef RENDER_OPENGL
  for (auto const& entry : std::filesystem::directory_iterator{ DATABASE_PATH }) {
    std::string path = entry.path().string();
    std::string str = path.substr(path.rfind("/") +1, path.rfind("."));

    if (str.compare(full_db) == 0) {
      return true;
    }
  }
#endif

  return false;
}

bool SceneManager::DeleteDB(std::string db_name){

  db_name.append(".db");
  std::string fullpath = DATABASE_PATH + db_name;

  return std::remove(fullpath.c_str());
}

