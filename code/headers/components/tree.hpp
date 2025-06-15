#ifndef __TREE_HPP_
#define __TREE_HPP_ 1

#include <defines.hpp>
#include <string>

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

/**
 * @brief Tree component that allows being parent/child of another component
 */
struct TreeComponent {
	//Friend declaration to update transformations 
	// through the tree component at the component manager
	friend struct ComponentManager;
protected:

	/** Parent component of the TreeComponent */
	size_t parent_;
	/** Total children of the TreeComponent */
	int num_children_;
	/** Children components of the TreeComponent */
	size_t children_[MAX_TREE_CHILDREN];
	char name[ENTITY_NAME_LENGTH];

public:
	TreeComponent();

	/**
	 * @brief Returns if the tree component has a parent
	 *
	 * @return bool True if the tree component has a parent, False if not
	 */
	inline bool HasParent() const { return parent_ != 0; }

	/**
	 * @brief Returns if the tree component has any child
	 *
	 * @return bool True if the tree component has any child, False if not
	 */
	inline bool HasChildren() const { return num_children_ > 0; }


	/**
	 * @brief 
	 */
	inline size_t GetParentID() const { return parent_; }

	/**
	 * @brief
	 */
	inline int GetNumChildren() const { return num_children_; }

	/**
	 * @brief Return the direction to the array of childrens, but const to prevent tampering
	 */
	inline const size_t* GetChildrensIDs() const { return children_; }

	 /**
	  * @brief Return the direction to the array of chars that is the name, but const to prevent tampering
	  */
	inline const char* GetEntityName() const { return name; }

	/**
	  * @brief Receive a the direction to the array of chars that is the name, but const to prevent tampering
	  * 
	  * @return bool True if the copy succeded or the string was empty
	  *				 False if the string was too long
	  */
	bool SetName(std::string new_name);
};

#endif


