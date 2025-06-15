#include "component_system.hpp"



// #### COMPONENT MANAGER ####

ComponentManager::ComponentManager() {
  num_entities_ = 0;
  draw_calls = 0;
  tree_has_changed_ = true;

  add_component_class<RendererComponent>();
  add_component_class<TransformComponent>();
  add_component_class<CameraComponent>();
  add_component_class<TreeComponent>();
}

size_t ComponentManager::new_entity() {
  size_t id = 0;
  //Check if deleted entities has values and retrieve the first
  if (deleted_entities_.size() != 0) {
    id = deleted_entities_.back();
    deleted_entities_.pop_back();
  }
  //If not, add one to the list and return it
  else { id = ++num_entities_; }

  tree_has_changed_ = true;

  //Add default components
  addComponent<TransformComponent>(id);
  TreeComponent* t = addComponent<TreeComponent>(id);
  sprintf_s(t->name, "Entity - %zd", id);

  return id;
}

EntityError ComponentManager::remove_entity(size_t e) {
  if (e == 0) { return EntityError::kInexistentEntity; }

  TreeComponent* tree_comp = get_component<TreeComponent>(e);

  //Delete all the childs recursively
  if (tree_comp != nullptr) {
    if (tree_comp->parent_!=0) {
      TreeComponent* parent_tree = get_component<TreeComponent>(tree_comp->parent_);
      bool found = false;
      for (int i = 0; !found && i < MAX_TREE_CHILDREN; i++) {

        if (parent_tree->children_[i] == e) {
          found = true;
          parent_tree->children_[i] = 0;
          parent_tree->num_children_--;
        }
      }
    }

    if (tree_comp->num_children_ > 0) {
      for (int i = 0; i < MAX_TREE_CHILDREN; i++) {
        if (tree_comp->children_[i] != 0) {
          remove_entity(tree_comp->children_[i]);
        }
      }
    }
  }

  //Delete every component associated with the entity removed from all the lists
  for (auto& [key, value] : components_classes_) {
    components_classes_.find(key)->second.get()->remove(e);		
  }

  deleted_entities_.emplace_back(e);

  std::sort(deleted_entities_.begin(), deleted_entities_.end());

  tree_has_changed_ = true;

  return EntityError::kOK;
}

void ComponentManager::set_entity_name(size_t e, std::string new_name){

    TreeComponent* t = get_component<TreeComponent>(e);

    if (t != nullptr) {
        strcpy_s(t->name, ENTITY_NAME_LENGTH, new_name.c_str());
    }
}

//## Inheritance methods
TreeComponentErrors ComponentManager::make_parent(size_t &parent_id, size_t &child_id) {

  static size_t tree_hash = typeid(TreeComponent).hash_code();

  //Check if they are the same id
  if(parent_id == child_id){return TreeComponentErrors::kCantParentItself;}

  //Check if both id's exist in the component manager
  if (parent_id <= 0 || parent_id > num_entities_ || child_id <=0 || child_id > num_entities_) {
    return TreeComponentErrors::kEntityIDNotFound;
  }
  

  //Get Inheritance component of both IDs
  TreeComponent* parent = get_component<TreeComponent>(parent_id);
  TreeComponent* child = get_component<TreeComponent>(child_id);

  if (parent == nullptr || child == nullptr) {return TreeComponentErrors::kEntityIsDeleted;}
  if (child->parent_ == parent_id) {return TreeComponentErrors::kOK;}
  if (parent->num_children_ == MAX_TREE_CHILDREN) {return TreeComponentErrors::kNoMoreChildrenSpace;}


  //If parent_id is higher than the child, swap them
  if (parent_id > child_id) {

    swap_entities(parent_id, child_id);
    size_t temp = parent_id;
    parent_id = child_id;
    child_id = temp;

    parent = get_component<TreeComponent>(parent_id);
    child = get_component<TreeComponent>(child_id);
  }
  
  //Get list of inherintances and iterate through it's IDs to find out if the parenting will cause a loop
  //If it finds the parent id, the child id, or repeats

  if (parent->parent_ != 0) {
    auto component_map_value = components_classes_.find(tree_hash)->second.get();
    auto& cl = *static_cast<component_list<TreeComponent>*>(component_map_value);
    bool incompatible = false;
    size_t first_id = parent_id;
    
    unsigned int num_comps= (unsigned int)cl.components_.size();

    for (TreeComponent* searched = parent; searched->parent_ != 0 && !incompatible;) {
      //Search in the parents
      if (searched->parent_ == first_id || 
          searched->parent_ == child_id || 
          searched->parent_ == parent_id) {
        incompatible = true;
      }
      else {
        //Find the Inheritance component of the searched->parent_
        for (unsigned int i = 0; i < num_comps; ++i) {
          if (cl.components_.at(i).entity_id_ == searched->parent_) {
            searched = &cl.components_.at(i).data_;
          }
        }
      }
    }

    if(incompatible){ return TreeComponentErrors::kParentingCreatesLoop;}
    
  }
  

  //Remove child in the list of the child's parent if different from 0
  if (child->parent_ != 0) {
    
    TreeComponent* child_parent = get_component<TreeComponent>(child->parent_);
    if (child_parent != nullptr) {
      bool found = false;
      for (int i = 0; !found && i < MAX_TREE_CHILDREN; ++i) {
        if (child_parent->children_[i] == child_id) {
          found = true;
          child_parent->children_[i] = 0;
          child_parent->num_children_--;
        }
      }
    }
  }

  //Add parent to child
  child->parent_ = parent_id;
  //Add children to parent by finding the first free space and using it
  bool found = false;
  for (int i = 0; !found && i < MAX_TREE_CHILDREN; ++i) {
    if (parent->children_[i] == 0) {
      found = true;
      parent->children_[i] = child_id;
      parent->num_children_++;
    }
  }

  tree_has_changed_ = true;

  //Add the transform matrix to the child and propagate
  TransformComponent* parent_trans = get_component<TransformComponent>(parent_id);
  TransformComponent* child_trans = get_component<TransformComponent>(child_id);

  if (nullptr != parent_trans && nullptr != child_trans) {

      child_trans->SetParentMatrix(parent_trans->GetTransform());

    UpdateChildTransforms(child_id);
  }


  return TreeComponentErrors::kOK;
}

TreeComponentErrors ComponentManager::make_child(size_t child_id, size_t parent_id) {
  return make_parent(parent_id, child_id);
}

TreeComponentErrors ComponentManager::remove_parent(size_t entity_id) {
  if(entity_id == 0 || entity_id> num_entities_){return TreeComponentErrors::kEntityIDNotFound;}

  //Search if deleted
  if (deleted_entities_.size() != 0) {
    for (unsigned int i = 0; i < deleted_entities_.size(); ++i) {
      if(deleted_entities_[i] == entity_id){
        return TreeComponentErrors::kEntityIDNotFound;
      }
    }
  }

  //Get inheritance component of parent and child
  TreeComponent* child = get_component<TreeComponent>(entity_id);
  if (child == nullptr) {return TreeComponentErrors::kEntityIsDeleted;}

  //Already parentless
  if(child->parent_ == 0) {return TreeComponentErrors::kOK;}

  TreeComponent* parent = get_component<TreeComponent>(child->parent_);

  //Search in parent to remove child
  bool found = false;
  for (unsigned int i = 0; !found && i < MAX_TREE_CHILDREN; ++i) {
    if (parent->children_[i] == entity_id) {
      found = true;
      parent->children_[i] = 0;
      parent->num_children_--;
    }
  }

  //Remove parent
  child->parent_ = 0;
  tree_has_changed_ = true;

  //Update the transform of this entitry and the childs to remove the reference to the parent
  TransformComponent* trans = get_component<TransformComponent>(entity_id);
  if (nullptr != trans) {
    trans->SetParentMatrix(glm::mat4(1.0f));
    UpdateChildTransforms(entity_id);
  }



  return TreeComponentErrors::kOK;
}

TreeComponentErrors ComponentManager::remove_children(size_t entity_id) {
  if (entity_id == 0 || entity_id > num_entities_) { return TreeComponentErrors::kEntityIDNotFound; }

  //Search if deleted
  if (deleted_entities_.size() != 0 &&
    std::find(deleted_entities_.begin(), deleted_entities_.end(), entity_id) == deleted_entities_.end()) {
    return TreeComponentErrors::kEntityIDNotFound;
  }

  //Get inheritance component of parent and child
  TreeComponent* parent = get_component<TreeComponent>(entity_id);
  if (parent == nullptr) { return TreeComponentErrors::kEntityIsDeleted; }

  //No children, no need
  if (parent->num_children_ == 0) {return TreeComponentErrors::kOK;}

  //Iterate through the children, and remove their fathers
  for (unsigned int i = 0; i < MAX_TREE_CHILDREN; ++i) {
    if(parent->children_[i]!=0){
      TreeComponent* children = get_component<TreeComponent>(parent->children_[i]);
      if (children != nullptr) {children->parent_ = 0;}

      parent->children_[i] = 0;
    }
  }

  //Set children to 0
  parent->num_children_ = 0;
  tree_has_changed_ = true;
  return TreeComponentErrors::kOK;
}

TreeComponentErrors ComponentManager::swap_entities(size_t first_id, size_t second_id){
  static size_t tree_hash = typeid(TreeComponent).hash_code();
  
  if(first_id == 0 || second_id == 0){ return TreeComponentErrors::kError;}
  if(first_id == second_id){ return TreeComponentErrors::kOK; }

  //Get tree component of both entities

  std::vector<component_node<TreeComponent>>* tree_comps_ =
    &(*static_cast<component_list<TreeComponent>*>(components_classes_.find(tree_hash)->second.get())).components_;

  component_node<TreeComponent>* first_tree = nullptr;
  component_node<TreeComponent>* second_tree = nullptr;

  for (unsigned int i = 0; (first_tree == nullptr || second_tree == nullptr) && i < tree_comps_->size(); ++i) {
    if (tree_comps_->at(i).entity_id_ == first_id) {
      first_tree = &(tree_comps_->at(i));
    }

    if (tree_comps_->at(i).entity_id_ == second_id) {
      second_tree = &(tree_comps_->at(i));
    }
  }

  if(first_tree == nullptr || second_tree == nullptr) { return TreeComponentErrors::kError; }

  
  
  //Update the childs of the first_tree
  if (first_tree->data_.HasChildren()) {
    for (unsigned int i = 0; i < MAX_TREE_CHILDREN; ++i) {
      if (first_tree->data_.children_[i] != 0) {
        get_component<TreeComponent>(first_tree->data_.children_[i])->parent_ = second_id;
      }
    }
  }
  //Update the parent of the first_tree
  if (first_tree->data_.parent_ != 0) {
    TreeComponent* first_parent_ = get_component<TreeComponent>(first_tree->data_.parent_);
    bool found = false;
    for (unsigned int i = 0; !false && i < MAX_TREE_CHILDREN; ++i) {
      if (first_parent_->children_[i] == first_id) {
        first_parent_->children_[i] = second_id;
        found = true;
      }
    }
  }
  //Update the component itself
  first_tree->entity_id_ = second_id;

  //Update the childs of the second_tree
  if (second_tree->data_.HasChildren()) {
    for (unsigned int i = 0; i < MAX_TREE_CHILDREN; ++i) {
      if (second_tree->data_.children_[i] != 0) {
        get_component<TreeComponent>(second_tree->data_.children_[i])->parent_ = first_id;
      }
    }
  }
  //Update the parent of the second_tree
  if (second_tree->data_.parent_ != 0) {
    TreeComponent* second_parent_ = get_component<TreeComponent>(second_tree->data_.parent_);
    bool found = false;
    for (unsigned int i = 0; !false && i < MAX_TREE_CHILDREN; ++i) {
      if (second_parent_->children_[i] == second_id) {
        second_parent_->children_[i] = first_id;
        found = true;
      }
    }
  }
  //Update the component itself
  second_tree->entity_id_ = first_id;

  //Sort the tree nodes
  std::sort(tree_comps_->begin(), tree_comps_->end());

  //Swap the components of each type but TreeComponent and sort the list
  for (auto& it : components_classes_) {
    if (it.first != tree_hash) {
      it.second.get()->swap_components(first_id, second_id);
    }
  }
  

  return TreeComponentErrors::kOK;
}

bool ComponentManager::IsMyChild(size_t search_id, size_t possible_child){

  bool is_parent = false;

  TreeComponent* maybe_parent_comp = get_component<TreeComponent>(possible_child);
  TreeComponent* search_id_comp = get_component<TreeComponent>(search_id);

  if (maybe_parent_comp == nullptr || search_id_comp == nullptr) {
    return false;
  }

  if (maybe_parent_comp->parent_ == 0) {return false;}

  //Search recursively through the parents until finding a root or the parent
  while (!is_parent && maybe_parent_comp->parent_ != 0) {
    if(maybe_parent_comp->parent_ == search_id){is_parent = true;}
    maybe_parent_comp = get_component<TreeComponent>(maybe_parent_comp->parent_);
  }

  return is_parent;
}

void ComponentManager::CheckChildTransformUpdates(){
    static size_t tree_hash = typeid(TreeComponent).hash_code();
    static size_t transf_hash = typeid(TransformComponent).hash_code();

    std::vector<component_node<TransformComponent>>* transform_components =
        &(*static_cast<component_list<TransformComponent>*>(components_classes_.find(transf_hash)->second.get())).components_;
    size_t transf_size = transform_components->size();

    TransformComponent* temp = nullptr;
    for (unsigned int i = 0; i < transf_size; ++i) {
        temp = &(transform_components->at(i).data_);
        //Update the child transforms of the given entity
        if (temp->updated_) {
            UpdateChildTransforms(transform_components->at(i).entity_id_);
        }
    }

}

void ComponentManager::UpdateChildTransforms(size_t parent_id){

    TreeComponent* parent = get_component<TreeComponent>(parent_id);
    TransformComponent* parent_transform = get_component<TransformComponent>(parent_id);

    if (nullptr == parent || nullptr == parent_transform) {return;}

    //Set the parent matrix of every child as the parent
    //Does nothing if there's no childs
    TransformComponent* child_transform;
    for (int i = 0; i < parent->num_children_; ++i) {
        child_transform = get_component<TransformComponent>(parent->children_[i]);

        //Some childs may not have a transform, such as cameras
        if (nullptr != child_transform) {

            //Update the parent matrix stored in the child
            //child_transform->parent = parent_transform->absolute;
            child_transform->SetParentMatrix(parent_transform->GetTransform());

            //Update the children transforms of the childrens
            UpdateChildTransforms(parent->children_[i]);
        }

    }

    //Mark as the parent has updated the childrens
    parent_transform->updated_ = false;

}

/** 
glm::mat4 ComponentManager::get_parent_transform_matrix(size_t entity_id) {

  static size_t tree_hash = typeid(TreeComponent).hash_code();
  static size_t transf_hash = typeid(TransformComponent).hash_code();

  glm::mat4 t = glm::mat4(1.0f);
  if (entity_id == 0) { return t; }

  //Form vector of parent entities
  std::vector<size_t> entities;
  //Pre-reserve a number of positions to avoid memory re-allocation in the loop
  entities.reserve(TRANSFORM_MATRIX_MEMORY_PRECACHE);

  std::vector<component_node<TreeComponent>>* tree_components =
    &(*static_cast<component_list<TreeComponent>*>(components_classes_.find(tree_hash)->second.get())).components_;
  std::vector<component_node<TransformComponent>>* transform_components =
    &(*static_cast<component_list<TransformComponent>*>(components_classes_.find(transf_hash)->second.get())).components_;
  size_t tree_size = tree_components->size();
  size_t transf_size = transform_components->size();

  size_t entity_to_find_ = entity_id;
  TreeComponent* temp_tree = nullptr;

  for (unsigned int i = 0; entity_to_find_ != 0 && i < tree_size; ++i) {

    if (tree_components->at(i).entity_id_ == entity_to_find_) {
      entities.emplace_back(entity_to_find_);
      temp_tree = &(tree_components->at(i).data_);
      //Reset the search if next searched id is less than actual ID
      if (entity_to_find_ > temp_tree->parent_) { i = 0; }

      //Mark next tree to find
      entity_to_find_ = temp_tree->parent_;
    }
  }

  //Create copy of the entities vector and reorder it
  unsigned int num_parents = (unsigned int)entities.size();
  std::vector<size_t> copy_entities;
  std::copy(entities.begin(), entities.end(), std::back_inserter(copy_entities));
  if (num_parents > 1) {
    std::sort(copy_entities.begin(), copy_entities.end());
  }

  //Get all transformations in the entities list
  std::map<size_t, glm::mat4> parent_transforms;
  //To find them, we check each entity against the first vector position of entities
  //When found (or not), the position is removed from the parent vector and the next is checked

  size_t temp_id = 0;
  for (size_t i = 0; copy_entities.size() != 0 && i < transf_size; ++i) {
    temp_id = copy_entities[0];
    if (transform_components->at(i).entity_id_ == temp_id) {

      //Get the transform
      parent_transforms[temp_id] = transform_components->at(i).data_.GetRelativeMatrix();

      //Remove the entity of the vector
      copy_entities.erase(copy_entities.begin());
    }

    //If last step and there are still entities remaining, reset counter and remove the entities[0] position because it's not found
    if (i + 1 == transf_size && copy_entities.size() != 0) {
      i = 0;
      copy_entities.erase(copy_entities.begin());
    }
  }

  //Apply the transformations in the right order
  for (size_t i = 0; i < num_parents; ++i) {
    t = parent_transforms[entities[i]] * t;
  }

  return t;
}
 */
//##

//## Methods to create prefilled entities
size_t ComponentManager::NewRenderer(){

  size_t id = new_entity();
  addComponent<RendererComponent>(id);

  TreeComponent* t = get_component<TreeComponent>(id);
  sprintf_s(t->name, "New Renderer - %zd", id);

  return id;
}

size_t ComponentManager::NewRenderer(std::shared_ptr<TinyObj>& mesh){

  size_t id = new_entity();
  addComponent<RendererComponent>(id)->Init(mesh);

  TreeComponent* t = get_component<TreeComponent>(id);
  strcpy_s(t->name, mesh->name_.c_str());

  return id;
}

size_t ComponentManager::NewOrthographicCamera() {
  size_t id = new_entity();
  addComponent<CameraComponent>(id)->SetOrthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 150.0f);

  TreeComponent* t = get_component<TreeComponent>(id);
  strcpy_s(t->name, "Orthographic Camera");

  return id;
}
size_t ComponentManager::NewPerspectiveCamera() {
  size_t id = new_entity();
  addComponent<CameraComponent>(id)->SetPerspective(90.0f, (16.0f/9.0f), 0.01f, 150.0f);
  TreeComponent* t = get_component<TreeComponent>(id);
  strcpy_s(t->name, "Perspective Camera");
  return id;
}

size_t ComponentManager::NewPerspectiveCamera(float fov, float aspect_ratio, float znear, float zfar) {
  size_t id = new_entity();
  addComponent<CameraComponent>(id)->SetPerspective(fov, aspect_ratio, znear, zfar);
  TreeComponent* t = get_component<TreeComponent>(id);
  strcpy_s(t->name, "Perspective Camera");
  return id;
}

//## Scene methods

CameraComponent* ComponentManager::get_principal_camera() {
  static size_t camera_hash = typeid(CameraComponent).hash_code();
  std::vector<component_node<CameraComponent>>* cameras = &(*static_cast<component_list<CameraComponent>*>(components_classes_.find(camera_hash)->second.get())).components_;
  unsigned int size = (unsigned int)cameras->size();

  if(size != 0){
    CameraComponent* c = nullptr;
    for(unsigned int i=0; c==nullptr && i<size;++i){
      if (cameras->at(i).data_.is_active_) {
        c = &(cameras->at(i).data_);
      }
    }

    return c;
  }
  
  return nullptr;
}

bool ComponentManager::update_tree(){

  static size_t tree_hash = typeid(TreeComponent).hash_code();

  if (!tree_has_changed_) {return false;}
  std::vector<component_node<TreeComponent>>* tree_comps_= 
    &(*static_cast<component_list<TreeComponent>*>(components_classes_.find(tree_hash)->second.get())).components_;
  
  //Clean tree before filling it again
  scene_tree_roots_.clear();

  for (size_t i = 0; i < tree_comps_->size(); ++i) {
      size_t id = tree_comps_->at(i).entity_id_;
      TreeComponent* t = &(tree_comps_->at(i).data_);
      if (t->parent_ == 0) {
        scene_tree_roots_.emplace_back(id);
      }
  }

  tree_has_changed_ = false;

  return true;
}

void ComponentManager::ResetComponentSystem() {
  //Remove all entities components and the entities itself
  for (auto& [key, value] : components_classes_) {
    components_classes_.find(key)->second.get()->clear_components();
  }
  deleted_entities_.clear();

  num_entities_ = 0;
  
}
void ComponentManager::Update(){

    //Update graph tree	        
    if (tree_has_changed_) {update_tree();}

    CheckChildTransformUpdates();

}
bool ComponentManager::CustomUnorderedMapContains(size_t val) {
    bool exists = false;

    for (auto& it : components_classes_) {
        if (it.first == val) {
            exists = true;
        }
    }

    return exists;
}

std::vector<size_t> ComponentManager::GetEntitiesByName(char* name){
    std::vector<size_t> entities;

    std::string searched_name = name;

    static size_t tree_hash = typeid(TreeComponent).hash_code();
    std::vector<component_node<TreeComponent>>* trees = &(*static_cast<component_list<TreeComponent>*>(components_classes_.find(tree_hash)->second.get())).components_;
    unsigned int size = (unsigned int)trees->size();
    
    if (size != 0) {
        TreeComponent* t = nullptr;
        for (unsigned int i = 0; t == nullptr && i < size; ++i) {
            t = &(trees->at(i).data_);

            //If the name matches, save the id
            if (searched_name.compare(t->name) == 0) {
                entities.push_back(trees->at(i).entity_id_);
            }
        }

    }

    return entities;
}
