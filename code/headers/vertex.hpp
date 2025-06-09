#ifndef __VERTEX_HPP__
#define __VERTEX_HPP__ 1

#include <glm/glm.hpp>

/**
 * @brief Structure that represents a simple vertex of a mesh
 */
struct Vertex {

  /** Position of the vertex */
  glm::vec3 position_;

  /** Color of the vertex */
  glm::vec3 color_;

  /** Normal of the vertex */
  glm::vec3 normal_;

  /** Texture Coordinates (UV) of the vertex */
  glm::vec2 uv_;

  static bool Compare(Vertex& v1, Vertex& v2) {
    if (v2.position_.x != v1.position_.x) { return false; }
    else if (v2.position_.y != v1.position_.y) { return false; }
    else if (v2.position_.z != v1.position_.z) { return false; }

    if (v2.uv_.x != v1.uv_.x) { return false; }
    else if (v2.uv_.y != v1.uv_.y) { return false; }

    if (v2.normal_.x != v1.normal_.x) { return false; }
    else if (v2.normal_.y != v1.normal_.y) { return false; }
    else if (v2.normal_.z != v1.normal_.z) { return false; }

    return true;
  }

  /**
   * @brief Operator equal
   * 
   * @param v1 Vertex to compare with
   * 
   * @return True if the vertex are equal, False if not.
   */
  bool operator==(const Vertex& v1) {
    if(position_.x != v1.position_.x){ return false; }
    else if(position_.y != v1.position_.y){ return false; }
    else if(position_.z != v1.position_.z){ return false; }

    if (uv_.x != v1.uv_.x) { return false; }
    else if (uv_.y != v1.uv_.y) { return false; }

    if (normal_.x != v1.normal_.x) { return false; }
    else if (normal_.y != v1.normal_.y) { return false; }
    else if (normal_.z != v1.normal_.z) { return false; }

    //No need to check colors, if the position does  match, this will too
    /*
    if (color_.x != v1.color_.x) { return false; }
    else if (color_.y != v1.color_.y) { return false; }
    else if (color_.z != v1.color_.z) { return false; }
    */

    return true;
  }
  
};



#endif //__VERTEX_HPP__