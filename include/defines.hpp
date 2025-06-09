#ifndef __DEFINES_HPP__
#define __DEFINES_HPP__ 1

/**
 * @brief Enumeration of the different types of shaders in the engine, for identification purposes
 */
enum class ShaderType {
  VERTEX,
  FRAGMENT,
  GEOMETRY
};

/**
 * @brief Enumeration of the different types of default shaders used for testing purposes
 */
enum class DefaultShaderSource {
  VERTEX,
  FRAGMENT_TEXTURE,
  FRAGMENT_NORMAL,
  FRAGMENT_UV,
  FRAGMENT_POSITION,
  FRAGMENT_PLAIN_COLOR
};

// #### SCENES DATABASES ####
#define GLSL_VERSION "#version 330"
#define DATABASE_CREATION_SQL_PATH "../data/db/scene_sqlite.sql"
#define DATABASE_PATH "../data/db/"
#define DATABASE_ALIAS_LENGTH 128


// #### LIGHTS ####
#define ADDITIVE_VERTEX_SHADER "../data/shaders_opengl/light_shader.vert"
#define ADDITIVE_FRAGMENT_SHADER "../data/shaders_opengl/additive_light_shader.frag"

#define DEPTHMAP_VERTEX_SHADER "../data/shaders_opengl/depthmap_shader.vert"
#define DEPTHMAP_FRAGMENT_SHADER "../data/shaders_opengl/depthmap_shader.frag"

#define DEPTHMAP_POINTLIGHT_FRAGMENT_SHADER "../data/shaders_opengl/depthmap_pointlight_shader.frag"
#define DEPTHMAP_POINTLIGHT_VERTEX_SHADER "../data/shaders_opengl/depthmap_pointlight_shader.vert"
#define DEPTHMAP_POINTLIGHT_GEOMETRY_SHADER "../data/shaders_opengl/depthmap_pointlight_shader.geom"

//#define MAX_DIRECTIONAL_LIGHTS 1
//#define MAX_POINT_LIGHTS 4
//#define MAX_SPOT_LIGHTS 4
#define MAX_TEXTURES 16

// #### CUBEMAP ####
#define CUBE_MESH_PATH "../data/meshes/cube.obj"
#define CUBEMAP_VERTEX_SHADER "../data/cubemap/shaders/cubemap_shader.vert"
#define CUBEMAP_FRAGMENT_SHADER "../data/cubemap/shaders/cubemap_shader.frag"
#define CUBEMAP_TEXTURES_FOLDER_PATH "../data/cubemap/textures"

// #### FORWARD RENDERING ####
#define RENDER_ELEMENTS_WITH_TEXTURE_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/elements_with_texture.vert"
#define RENDER_ELEMENTS_WITH_TEXTURE_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/elements_with_texture.frag"

#define RENDER_DIRECTIONAL_AND_SPOTLIGHT_SHADOWS_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/directional_and_spotlight_shadow.vert"
#define RENDER_DIRECTIONAL_AND_SPOTLIGHT_SHADOWS_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/directional_and_spotlight_shadow.frag"

#define RENDER_DIRECTIONAL_ELEMENTS_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/directional_elements.vert"
#define RENDER_DIRECTIONAL_ELEMENTS_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/directional_elements.frag"

#define RENDER_SPOTLIGHT_ELEMENTS_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/spotlight_elements.vert"
#define RENDER_SPOTLIGHT_ELEMENTS_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/spotlight_elements.frag"

#define RENDER_POINTLIGHT_SHADOWS_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/pointlight_shadow.vert"
#define RENDER_POINTLIGHT_SHADOWS_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/pointlight_shadow.frag"
#define RENDER_POINTLIGHT_SHADOWS_GEOMETRY_SHADER "../data/shaders_opengl/forward_rendering/pointlight_shadow.geom"

#define RENDER_POINTLIGHT_ELEMENTS_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/pointlight_elements.vert"
#define RENDER_POINTLIGHT_ELEMENTS_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/pointlight_elements.frag"

#define RENDER_FRAME_BUFFER_TO_SCREEN_VERTEX_SHADER "../data/shaders_opengl/forward_rendering/frame_buffer_to_screen.vert"
#define RENDER_FRAME_BUFFER_TO_SCREEN_FRAGMENT_SHADER "../data/shaders_opengl/forward_rendering/frame_buffer_to_screen.frag"

#define SHADOWS_DIMENSIONS 2048

// #### DEFERRED RENDERING ####
#define DEFERRED_GEOMETRY_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/gbuffer_shader.vert"
#define DEFERRED_GEOMETRY_FRAGMENT_SHADER "../data/shaders_opengl/deferred_rendering/gbuffer_shader.frag"

#define DEFERRED_DIRECTIONAL_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/deferred_directional_shader.vert"
#define DEFERRED_DIRECTIONAL_FRAGMENT_SHADER "../data/shaders_opengl/deferred_rendering/deferred_directional_shader.frag"

#define DEFERRED_SPOTLIGHT_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/deferred_spotlight_shader.vert"
#define DEFERRED_SPOTLIGHT_FRAGMENT_SHADER "../data/shaders_opengl/deferred_rendering/deferred_spotlight_shader.frag"

#define DEFERRED_POINTLIGHT_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/deferred_pointlight_shader.vert"
#define DEFERRED_POINTLIGHT_FRAGMENT_SHADER "../data/shaders_opengl/deferred_rendering/deferred_pointlight_shader.frag"

#define DEFERRED_ELEMENTS_WITH_TEXTURE_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/deferred_rendering_shader.vert"
#define DEFERRED_ELEMENTS_WITH_TEXTURE_FRAGMENT_SHADER "../data/shaders_opengl/deferred_rendering/deferred_rendering_shader.frag"

#define DEFERRED_POSTPROCESS_VERTEX_SHADER "../data/shaders_opengl/postprocesses/postprocess_shader.vert"
#define DEFERRED_POSTPROCESS_FRAGMENT_SHADER "../data/shaders_opengl/postprocesses/postprocess_shader.frag"

#define MAX_TREE_CHILDREN 10
#define TRANSFORM_MATRIX_MEMORY_PRECACHE 100

#define ENTITY_NAME_LENGTH 128

#define MESHES_FOLDER_PATH "../data/meshes/"
#define TEXTURES_FOLDER_PATH "../data/textures/"

// #### TEXT RENDERING ####
#define TEXT_RENDERING_VERTEX_SHADER "../data/shaders_opengl/deferred_rendering/text_render.vert"
#define TEXT_RENDERING_PIXEL_SHADER "../data/shaders_opengl/deferred_rendering/text_render.frag"
#define TEXT_RENDERING_MAX_SIZE	128

// #### DIRECTX SHADERS ###

#define DIRECTX_PIXEL_SHADER_VERSION "ps_4_1"
#define DIRECTX_VERTEX_SHADER_VERSION "vs_4_1"

#endif // __DEFINES_HPP__