#include <tree.hpp>

// #### TREE COMPONENT ####

TreeComponent::TreeComponent() {
    parent_ = 0;
    num_children_ = 0;
    for (int i = 0; i < MAX_TREE_CHILDREN; ++i) {
        children_[i] = 0;
    }

    name[0] = '\0';
}

bool TreeComponent::HasParent() {
    return parent_ != 0;
}

bool TreeComponent::HasChildren() {
    return num_children_ > 0;
}

