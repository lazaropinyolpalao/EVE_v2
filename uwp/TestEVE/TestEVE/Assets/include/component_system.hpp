#ifndef __COMPONENTS_HPP__
#define __COMPONENTS_HPP__ 1

#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <typeinfo>
#include <mutex>
#include <algorithm>

#include <iostream>
#include <stdlib.h>

//GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

#include <window.hpp>
#include <texture.hpp>
#include <tinyobj.hpp>
#include <light.hpp>
#include <cubemap.hpp>
#include <framebuffer_to_texture.hpp>
#include <deferred_framebuffer.hpp>
#include <depth_map.hpp>

/**
 * @brief Enumeration of the different types of errors that an entity can cause
 */
enum class EntityError {
	kOK,
	kError,
	kInexistentEntity,
	kAlreadyCreatedComponent,
};

/**
 * @brief Enumeration of the different types of errors that the tree component can cause
 */
enum class TreeComponentErrors {
	kOK,
	kCantParentItself,
	kError,
	kEntityIDNotFound,
	kEntityIsDeleted,
	kRepeatedEntity,
	kHasNoParent,
	kHasNoChildren,
	kNoMoreChildrenSpace,
	kParentingCreatesLoop
};



// #### ECS COMPONENTS ####

/**
 * @brief Transformation component that gives an object scale, rotation and translation
 */
struct TransformComponent {

	/** Main matrix of the transform component */
	glm::mat4 t;
	/** Scale vector of the transform component */
	glm::vec3 scale_;
	/** Rotation vector of the transform component */
	glm::vec3 rotation_;
	/** Position vector of the transform component */
	glm::vec3 position_;
	/** Wheter the data of the transform component has changed or not, used to update the main matrix of the transform component */
	bool changed_;

	TransformComponent();

	/**
	 * @brief Gets the main matrix of the transform component
	 * 
	 * @return glm::mat4 The main matrix of the transform component 
	 */
	glm::mat4 GetMatrix();

	/**
	 * @brief Sets the scale of the transform component
	 * 
	 * @param x Desired scale for the X axis
	 * @param y Desired scale for the Y axis
	 * @param z Desired scale for the Z axis
	 * 
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetScale(float x, float y, float z);

	/**
	 * @brief Sets the scale of the transform component
	 * 
	 * @param scl A vector with the desired scale for the X, Y and Z axis
	 * 
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetScale(glm::vec3 scl);


	/**
	 * @brief Adds a rotation to the X axis of the transform component
	 * 
	 * @param x Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 * 
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationX(float x, bool radians = false);

	/**
	 * @brief Adds a rotation to the Y axis of the transform component
	 *
	 * @param y Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationY(float y, bool radians = false);

	/**
	 * @brief Adds a rotation to the Z axis of the transform component
	 *
	 * @param z Desired angle to add
	 * @param radians Whether the angle is given in radians or degrees
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* AddRotationZ(float z, bool radians = false);

	/**
 * @brief Adds a rotation to the X, Y and Z axis of the transform component
 *
 * @param x Desired angle to add to the X axis
 * @param y Desired angle to add to the Y axis
 * @param z Desired angle to add to the Z axis
 * @param radians Whether the angle is given in radians or degrees
 *
 * @return TransformComponent* A pointer to the same modified TransformComponent
 */
	TransformComponent* AddRotation(float x, float y, float z, bool radians = false);

	/**
 * @brief Adds a rotation to the X, Y and Z axis of the transform component
 *
 * @param x Desired angle to add to the X axis
 * @param y Desired angle to add to the Y axis
 * @param z Desired angle to add to the Z axis
 * @param radians Whether the angle is given in radians or degrees
 *
 * @return TransformComponent* A pointer to the same modified TransformComponent
 */
	TransformComponent* AddRotation(glm::vec3 rot, bool radians = false);


	/**
	 * @brief Sets the position of the transform component
	 * 
	 * @param x Desired position for the X axis
	 * @param y Desired position for the Y axis
	 * @param z Desired position for the Z axis
	 * 
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetTranslation(float x, float y, float z);

	/**
	 * @brief Sets the position of the transform component
	 *
	 * @param tsl A vector with th desired position for the X, Y and Z axis
	 *
	 * @return TransformComponent* A pointer to the same modified TransformComponent
	 */
	TransformComponent* SetTranslation(glm::vec3 tsl);

};


/**
 * @brief Renderer component that allows an entity to be drawn
 */
struct RendererComponent {

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

/**
 * @brief Enumeration of the different camera modes
 */
enum class CameraMode {
	kPerspective, 
	kOrthographic
};

/**
 * @brief Camera component that provides a projection of the scene
 */
struct CameraComponent {

	/** Whether the camera is active or not */
	bool is_active_;

	//Shared elements
	/** Active camera mode */
	CameraMode mode_;

	//X, Y, Z
	/** Angle of rotation on the X axis */
	float pitch_;
	/** Angle of rotation on the Y axis */
	float yaw_;
	/** Angle of rotation on the Z axis */
	float roll_;

	/** Position of the camera */
	glm::vec3 position_;
	/** Rotation of the camera */
	//glm::mat4 rotation_;

	/** Forward direction of the camera */
	//glm::vec3 forward_dir_;
	/** Right direction of the camera */
	//glm::vec3 right_dir_;
	/** Up direction of the camera */
	//glm::vec3 up_dir_;

	/** Projection matrix of the camera */
	glm::mat4 projection_;
	/** View matrix of the camera */
	glm::mat4 view_;

	/** Nearest distance where the camera will start its projection */
	float znear_;
	/** Farest distance where the camera will stop its projection */
	float zfar_;

	//Orthographic
	/** Left expansion of the orthographic camera */
	float left_;
	/** Right expansion of the orthographic camera */
	float right_;
	/** Bottom expansion of the orthographic camera */
	float bottom_;
	/** Top expansion of the orthographic camera */
	float top_;

	//Perspective
	/** Field of view of the perspective camera */
	float fov_;
	/** Field of view of the perspective camera */
	float aspect_ratio_;
	
	CameraComponent();
	CameraComponent(CameraMode mode);

	/**
	 * @brief Sets the camera as an orthographic camera
	 *  
	 * @param l Left expansion of the orthographic camera
	 * @param r Right expansion of the orthographic camera
	 * @param b Bottom expansion of the orthographic camera
	 * @param t Top expansion of the orthographic camera
	 * @param zn Near distance of the camera
	 * @param zf Far distance of the camera
	 * 
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* SetOrthographic(float l, float r, float b, float t, float zn, float zf);

	/**
	 * @brief Sets the camera as a perspective camera
	 * 
	 * @param f Field of view of the perspective camera
	 * @param ar Aspect ration of the perspective camera
	 * @param zn Near distance of the camera
	 * @param zf Far distance of the camera
	 * 
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* SetPerspective(float f, float ar, float zn, float zf);

	/**
	 * @brief Updates the values of the camera when receives an input
	 * 
	 * @param i Input to process
	 * @param dt Elapsed time between one frame and another
	 * @param speed Camera movement speed
	 * 
	 * @return CameraComponent* A pointer to the same modified CameraComponent
	 */
	CameraComponent* UpdateCamera(Input i, float dt, float speed = 0.25f);
};

/**
 * @brief Tree component that allows being parent/child of another component
 */
struct TreeComponent {

	/** Parent component of the TreeComponent */
	size_t parent_;
	/** Total children of the TreeComponent */
	int num_children_;
	/** Children components of the TreeComponent */
	size_t children_[MAX_TREE_CHILDREN];

	TreeComponent();

	/**
	 * @brief Returns if the tree component has a parent
	 * 
	 * @return bool True if the tree component has a parent, False if not 
	 */
	bool HasParent();

	/**
	 * @brief Returns if the tree component has any child
	 * 
	 * @return bool True if the tree component has any child, False if not
	 */
	bool HasChildren();

};


/**
 * @brief Node of the ECS that encapsules an entity id and a component
 */
template<typename T>
struct component_node {

	/** Entity identifier */
	size_t entity_id_;
	/** Node data, stores a component*/
	T data_;

	/**
	 * @brief Operator less than
	 * 
	 * @param c Component node to compare with
	 * 
	 * @return bool True if the component id is less than the compared one, False if not
	 */
	bool operator<(const component_node& c) { return entity_id_ < c.entity_id_; }

	component_node(){entity_id_ = 0;}
};

/**
 * @brief Base component of the ECS
 */
struct component_base {

	/**
	 * @brief Expands the components list by one
	 * 
	 * @param e Entity that expand the list of components
	 */
	virtual void grow(size_t e) = 0;

	/**
	 * @brief Returns the size of the components list
	 * 
	 * @return size_t The size of the components list
	 */
	virtual size_t size() = 0;

	/**
	 * @brief Removes a component from a specific entity
	 * 
	 * @param e Entity to remove the component from
	 * 
	 * @return bool True if the component is found and removed, False if not
	 */
	virtual bool remove(size_t e) = 0;

	/**
	 * @brief Swaps a component of two given entities
	 * 
	 * @param first First entity to swap component
	 * @param second Second entity to swap component
	 * 
	 * @return bool True if the component is found in both entities and swapped, False if not
	 */
	virtual bool swap_components(size_t first, size_t second) = 0;

	/**
	 * @brief Clear all the components list
	 * 
	 */
	virtual void clear_components() = 0;

};

/**
 * @brief Inherits from component_base and implements the methods alongside a vector of component_node elements
 */
template<typename T>
struct component_list : component_base {
	/** Vector of component_node<T> elements that holds a component of T type off an entity */
	std::vector<component_node<T>> components_;

	/**
	 * @brief Expands the components list by one
	 *
	 * @param e Entity that expand the list of components
	 */
	virtual void grow(size_t e) {
		components_.emplace_back();
		components_.at(components_.size()-1).entity_id_ = e;
	}

	/**
	 * @brief Returns the size of the components list
	 *
	 * @return size_t The size of the components list
	 */
	virtual size_t size() {return components_.size();}

	/**
	 * @brief Removes a component from a specific entity
	 *
	 * @param e Entity to remove the component from
	 *
	 * @return bool True if the component is found and removed, False if not
	 */
	virtual bool remove(size_t e) {

		bool found = false;
		size_t pos = 0;
		for (size_t i = 0; !false && i < components_.size(); ++i) {
			if(components_.at(i).entity_id_ == e){ 
				found = true;
				pos = i;
			}
		}

		if (found) {
			components_.erase(components_.begin()+(pos));
			return true;
		}

		return false;
	}

	/**
	 * @brief Swaps a component of two given entities
	 *
	 * @param first First entity to swap component
	 * @param second Second entity to swap component
	 *
	 * @return bool True if the component is found in both entities and swapped, False if not
	 */
	virtual bool swap_components(size_t first, size_t second) {

		//Get reference to both components
		component_node<T>* firstComp = nullptr;
		component_node<T>* secondComp = nullptr;

		for (size_t i = 0; (firstComp == nullptr || secondComp == nullptr) && i < components_.size(); ++i) {
			if (components_.at(i).entity_id_ == first) {firstComp = &(components_.at(i));}
			if (components_.at(i).entity_id_ == second) { secondComp = &(components_.at(i));}
		}

		//Change ids
		if (firstComp == nullptr && secondComp == nullptr) {return false;}

		if (firstComp != nullptr) { firstComp->entity_id_ = second;}
		if (secondComp != nullptr) { secondComp->entity_id_ = first;}

		//Reorder list
		std::sort(components_.begin(), components_.end());

		return true;
	}

	/**
	 * @brief Clear all the components list
	 *
	 */
	virtual void clear_components() {components_.clear();}
};

/**
 * @brief Struct used to manage the creation of entities and their components, and also the resources used alongisde the entities like lights, meshes and textures
 */
struct ComponentManager {
	/** Unordered map of each type of component, that holds all components of the same class, to faster usage */
	std::unordered_map<size_t, std::unique_ptr<component_base>> components_classes_;
	/** Counter of actual entities, including the deleted ones */
	size_t num_entities_;
	/** Vector of deleted entities. When creating a new entity, it's id is first retrieved from one on here */
	std::vector<size_t> deleted_entities_;
	/** Struct that holds all resources used alongside the entities, like lights, meshes, and textures */
	//Resources resource_list_;

	ComponentManager();

	//## Component methods ##
	/**
	 * @brief Adds a new class of type T to the component_classes_ map
	 */
	template<typename T> 
	void add_component_class() {
		size_t hash = typeid(T).hash_code();

#if __cplusplus > 201703L
		if (components_classes_.contains(hash)) return;
#else 
		if (CustomUnorderedMapContains(hash)) return;
#endif


		components_classes_.insert({ hash, std::make_unique<component_list<T>>() });
	}
	
	/**
	 * @brief Adds a component of type T associated to an entity if it exists 
	 * 
	 * @param e Entity id
	 * @return T* pointer to the component
	 */
	template<typename T>
	T* addComponent(size_t e) {
		//Select the right component_list to search if the component already exists
		auto hc = typeid(T).hash_code();

#if __cplusplus > 201703L
	if (components_classes_.size() == 0 || !components_classes_.contains(hc) || e == 0) {
		return nullptr;
	}
#else 
	if (components_classes_.size() == 0 || !CustomUnorderedMapContains(hc) || e == 0) {
		return nullptr;
	}
#endif

		


		auto component_map_value = components_classes_.find(hc)->second.get();
		auto& cl = *static_cast<component_list<T>*>(component_map_value);
		//Search if the entity doesn't have already this component
		bool found = false;
		T* temp = nullptr;
		for (int i = 0; !found && i < cl.components_.size(); ++i) {
			if (cl.components_.at(i).entity_id_ == e) {
				found = true;
				temp = &cl.components_.at(i).data_;
			}
		}
		if (found) { return temp; }

		//Add a new position to the vector with the given entity id
		size_t prev_id = 0;
		if (cl.components_.size() != 0) {
			prev_id = cl.components_[cl.components_.size() - 1].entity_id_;
		}
		
		component_map_value->grow(e);

		//If new id less than last of vector, rearrange vector
		if (e < prev_id) {
			std::sort(cl.components_.begin(), cl.components_.end());
		}

		found = false;
		for (int i = 0; !found && i < cl.components_.size(); ++i) {
			if (cl.components_.at(i).entity_id_ == e) {
				found = true;
				temp = &cl.components_.at(i).data_;
			}
		}
		return temp;
	}

	/**
	 * @brief Retrieve a T* component associated to an entity if it exists
	 * 
	 * @param e Entity id
	 * @return T* pointer to the component
	 */
	template<typename T>
	T* get_component(size_t e) {

		//Select the right component_list to search the component
		auto hc = typeid(T).hash_code();

#if __cplusplus > 201703L
	if (components_classes_.size() == 0 || !components_classes_.contains(hc) || e == 0) {
		return nullptr;
	}
#else 
	if (components_classes_.size() == 0 || !CustomUnorderedMapContains(hc) || e == 0) {
		return nullptr;
	}
#endif

		auto component_map_value = components_classes_.find(hc)->second.get();
		auto& cl = *static_cast<component_list<T>*>(component_map_value);

		size_t comp_size_ = cl.components_.size();
		if (comp_size_ == 0) { return nullptr; }

		//Search for the element through the list
		bool found = false;
		T* comp = nullptr;

		//First search directly on that spot if no 
		//deleted entities and the id is less than the size of the components
		if (deleted_entities_.size() == 0 && comp_size_ > e && cl.components_.at(e - 1).entity_id_ == e) {
			found = true;
			comp = &cl.components_.at(e - 1).data_;
		}
		else {

			//Binary search through the elements
			if (comp_size_ > 100) {
				//Limit of searches and count
				unsigned int max_search_num = (unsigned int)(comp_size_ * 0.5f);
				unsigned int searches = 0;
				//Starting point of search
				unsigned int find = max_search_num;
				unsigned int increments = find;

				unsigned int min = 0, max = (unsigned int)(comp_size_ - 1);

				//If the found is the searched, stop and quit, if not, 
				//change find position according to value and restart

				while (comp == nullptr && searches < max_search_num && find >= min && find <= max) {

					size_t id = cl.components_[find].entity_id_;

					//Stablish next jump size, with a minimum of 1
					increments = (unsigned int)(increments * 0.5f);
					if (increments == 0) { increments = 1; }

					if (id == e) { comp = &cl.components_[find].data_; }
					//If the found id is lower than the searched, search up by half
					else if (id < e) { find += increments; }
					//If the found id is higher than the searched, search down by half
					else { find -= increments; }

					searches++;
				}

			}
			//Search from the start
			else {
				for (int i = 0; !found && i < comp_size_; ++i) {
					if (cl.components_.at(i).entity_id_ == e) {
						found = true;
						comp = &cl.components_.at(i).data_;
					}
				}
			}
		}

		return comp;
	}

	/**
	 * @brief Retrieve the id associated with a en entity that belongs to a component
	 * 
	 * @param comp Component to search with
	 * @return size_t Id of the associated entity
	 */
	template<typename T>
	size_t get_id_of_component(T* comp) {
		size_t id = 0;
		auto hc = typeid(*comp).hash_code();
		if (components_classes_.size() == 0 || !components_classes_.contains(hc)) {
			return id;
		}

		auto component_map_value = components_classes_.find(hc)->second.get();
		auto& cl = *static_cast<component_list<T>*>(component_map_value);

		bool found = false;
		//Iterate through the components to find it
		for (size_t i = 0; !found && i < cl.components_.size(); ++i) {
			if (&cl.components_.at(i).data_ == comp) {
				id = cl.components_.at(i).entity_id_;
				found = true;
			}
		}

		return id;
	}

	bool CustomUnorderedMapContains(size_t val);

	//##

	//## Entity methods ##
	/**
	 * @brief Create new entity and return it's associated id
	 * 
	 * @return size_t Id of the entity
	 */
	size_t new_entity();

	//TODO: Remove children too or unparent them if any
	/**
	 * @brief Remove an entity and it's associated components
	 *
	 * @param size_t Id of the entity
	 * @return EntityError Error produced if any
	 */
	EntityError remove_entity(size_t e);

	//##

	//##Inheritance methods
	/** Wheter the tree has changed and needs to be updated or not */
	bool tree_has_changed_;
	/** Entites that don't have a parent*/
	std::vector<size_t> scene_tree_roots_;

	/**
	 * @brief Make an entity child of another 
	 * 
	 * @param child_id Entity to make children
	 * @param parent_id Parent entity
	 * @return TreeComponentErorrs Error, if any
	 */
	TreeComponentErrors make_parent(size_t &parent_id, size_t &child_id);

	/**
	* @brief Make an entity child of another through it's components
	*
	* @param child_comp Child component
	* @param parent_comp Parent component
	* @return TreeComponentErorrs Error, if any
	*/
	template<typename TP, typename TC>
	TreeComponentErrors make_parent(TP* parent_comp, TC* child_comp) {

		if(parent_comp == child_comp){ return TreeComponentErrors::kCantParentItself;}

		size_t id_parent = get_id_of_component(parent_comp);
		size_t id_child = get_id_of_component(child_comp);
		if (id_parent == 0 || id_child == 0) {return TreeComponentErrors::kEntityIDNotFound;}
		if (id_parent == id_child){ return TreeComponentErrors::kCantParentItself;}

		return make_parent(id_parent, id_child);
	}

	/**
	 * @brief Make an entity child of another
	 * 
	 * @param child_id Entity to make children
	 * @param parent_id Parent entity
	 * @return TreeComponentErorrs Error, if any
	 */
	TreeComponentErrors make_child(size_t child_id, size_t parent_id);

	/**
	 * @brief Make an entity child of another through it's components
	 *
	 * @param child_comp Child component
	 * @param parent_comp Parent component
	 * @return TreeComponentErorrs Error, if any
	 */
	template<typename TC, typename TP>
	TreeComponentErrors make_child(TC* child_comp, TP* parent_comp) {
		return make_parent(parent_comp, child_comp);
	}

	/**
	 * @brief Remove the parent of an entity, if it has
	 * 
	 * @param entity_id Id of the entity to unparent
	 * @return TreeComponentErorrs Error, if any
	 */
	TreeComponentErrors remove_parent(size_t entity_id);

	/**
	 * @brief Revemove the parent of an entity, if it has, through it's component
	 * 
	 * @param component The component of an entity to search and remove parent from
	 * @return TreeComponentErorrs Error, if any
	 */
	template<typename T>
	TreeComponentErrors remove_parent(T* component) {
		size_t id = get_id_of_component(component);

		if(id ==0) { return TreeComponentErrors::kEntityIDNotFound; }
		
		return remove_parent(id);
	}

	/**
	 * @brief Remove the children of an entity, if it has
	 * 
	 * @param entity_id The id to remove the children from
	 * @return TreeComponentErorrs Error, if any
	 */
	TreeComponentErrors remove_children(size_t entity_id);

	/**
	 * @brief Remove the children of an entity, if it has, through it's component
	 * 
	 * @param component The component of an enitity to search and remove parent from
	 * @return TreeComponentErorrs Error, if any
	 */
	template<typename T>
	TreeComponentErrors remove_children(T* component) {
		size_t id = get_id_of_component(component);

		if (id == 0) { return TreeComponentErrors::kEntityIDNotFound; }
		return remove_children(id);

	}
	
	/**
	 * @brief Swap the components of two entities and update the childrens and parents
	 * 
	 * @param first_id Id of the first entity to swap it's components with the second
	 * @param second_id Id of the second entity to swap it's components with the first
	 * @return TreeComponentErorrs Error, if any
	 */
	TreeComponentErrors swap_entities(size_t first_id, size_t second_id);

	/**
	 * @brief Checks if a given entity has a along the tree that matches a given entity
	 * 
	 * @param search_id The entity to search from
	 * @param possible_child The entity to search upwards from
	 * @return Wheter it's a tree parent or not, or false if one of the entities is deleted or doesn't have the component
	 */
	bool IsMyChild(size_t search_id, size_t possible_child);

	/**
	 * @brief Find all parent transformations of an entity and return the resulting matrix
	 * 
	 * @param entity_id Which entity to retrieve the transformations
	 * @return glm::mat4 Final transformation matrix
	 */
	glm::mat4 get_parent_transform_matrix(size_t entity_id);
	//##

	//Methods that return an entity with already added components, eg:
	/**
	 * @brief Create and return an entity with already added Transform and Renderer components
	 * 
	 * @return size_t The id of the newly created entity
	 */
	size_t NewRenderer();
	/**
	* @brief Create and return an entity with already added Transform and Renderer components
	* 
	* @param mesh The mesh to associate the render with
	* @return size_t The id of the newly created entity
	*/
	size_t NewRenderer(std::shared_ptr<TinyObj>& mesh);

	/**
	 * @brief Create and return an entity with already added Camera component and seted as a OrthographicCamera
	 * 
	 * @return size_t The id of the newly created entity
	 */
	size_t NewOrthographicCamera();

	/**
	* @brief Create and return an entity with already added Camera component and seted as a PerspectiveCamera
	*
	* @return size_t The id of the newly created entity
	*/
	size_t NewPerspectiveCamera();

	/**
	* @brief Create and return an entity with already added Camera component and seted as a PerspectiveCamera
	*
	* @param fov	Field of view
	* @param aspect_ratio Aspect ratio
	* @ param znear Near value of the camera
	* @ param zfar Far value of the camera
	* @return size_t The id of the newly created entity
	*/
	size_t NewPerspectiveCamera(float fov, float aspect_ratio, float znear, float zfar);

	//## Systems methods ##
	/**
	 * @brief Retrieve the camera component of the first camera of the ECS marked as active
	 * 
	 * @return CameraComponent* Pointer to the active camera, if any, nullptr if none
	 */
	CameraComponent* get_principal_camera();

	/**
	 * @brief Update the graph scene tree rots
	 * 
	 * @return bool If the three has been updated or not
	 */
	bool update_tree();

	/**
	 * @brief Removes all entities, their components, and frees al resources from the component manager
	 */
	void ResetComponentSystem();


	//For debug information
	/** Number of draw calls of the frame */
	unsigned int draw_calls;
};


#endif // __COMPONENTS_HPP__