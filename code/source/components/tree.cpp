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
//
//inline bool TreeComponent::HasParent() const {return parent_ != 0;}
//
//inline bool TreeComponent::HasChildren() const {return num_children_ > 0;}
//
//inline size_t TreeComponent::GetParentID() const {return parent_;}
//
//inline int TreeComponent::GetNumChildren() const{return num_children_;}
//
//inline const size_t* TreeComponent::GetChildrensIDs() const{return children_;}
//
//inline const char* TreeComponent::GetEntityName() const{return name;}

bool TreeComponent::SetName(std::string new_name){
    if (new_name.size() > ENTITY_NAME_LENGTH) { return false; }

    //Delete all the characters saved first
    for (int i = 0; i < ENTITY_NAME_LENGTH; ++i) {name[i] = '\0';}

    //Copy the new characters until the limit of the space or the string, whichever comes first
    for (int i = 0; i < new_name.size() && i < ENTITY_NAME_LENGTH; ++i) {
        name[i] = new_name[i];
    }

    return true;
}
