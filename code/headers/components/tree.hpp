#ifndef __TREE_HPP_
#define __TREE_HPP_ 1


#include <defines.hpp>

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
 * @brief Tree component that allows being parent/child of another component
 */
struct TreeComponent {

	/** Parent component of the TreeComponent */
	size_t parent_;
	/** Total children of the TreeComponent */
	int num_children_;
	/** Children components of the TreeComponent */
	size_t children_[MAX_TREE_CHILDREN];
	char name[ENTITY_NAME_LENGTH];

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

#endif
