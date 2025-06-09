CREATE TABLE IF NOT EXISTS textures(
  texture_id INTEGER PRIMARY KEY,
  texture_src TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS meshes(
  mesh_id INTEGER PRIMARY KEY,
  mesh_src TEXT NOT NULL,
  cull_type INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS screen_text(
  screen_text_id INTEGER PRIMARY KEY,
  text_ TEXT NOT NULL,
  color_x REAL, color_y REAL, color_z REAL,
  pos_x REAL,
  pos_y REAL,
  scale REAL
);

CREATE TABLE IF NOT EXISTS directional_light(
  directional_id INTEGER PRIMARY KEY,
  visible INTEGER NOT NULL,
  position_x REAL, position_y REAL, position_z REAL,
  ambient_x REAL, ambient_y REAL, ambient_z REAL,
  diffuse_x REAL, diffuse_y REAL, diffuse_z REAL,
  specular_x REAL, specular_y REAL, specular_z REAL,
  znear REAL, zfar REAL,
  left_ REAL, right_ REAL, bottom_ REAL, top_ REAL, 
  yaw REAL,
  pitch  REAL
);

CREATE TABLE IF NOT EXISTS spot_light(
  spot_id INTEGER PRIMARY KEY,
  visible INTEGER NOT NULL,
  position_x REAL, position_y REAL, position_z REAL,
  ambient_x REAL, ambient_y REAL, ambient_z REAL,
  diffuse_x REAL, diffuse_y REAL, diffuse_z REAL,
  specular_x REAL, specular_y REAL, specular_z REAL,
  znear REAL, zfar REAL,
  aspect_ratio REAL,
  fov REAL, 
  yaw REAL,
  pitch REAL,
  cut_off REAL,
  outer_cut_off REAL,
  constant REAL,
  linear REAL,
  quadratic REAL,
  range REAL
);

CREATE TABLE IF NOT EXISTS point_light(
  point_id INTEGER PRIMARY KEY,
  visible INTEGER NOT NULL,
  position_x REAL, position_y REAL, position_z REAL,
  ambient_x REAL, ambient_y REAL, ambient_z REAL,
  diffuse_x REAL, diffuse_y REAL, diffuse_z REAL,
  specular_x REAL, specular_y REAL, specular_z REAL,
  znear REAL, zfar REAL,
  aspect_ratio REAL,
  constant REAL,
  linear REAL,
  quadratic REAL,
  range REAL
);

CREATE TABLE IF NOT EXISTS entities(entity_id INTEGER PRIMARY KEY);

CREATE TABLE IF NOT EXISTS transform(
  transform_id INTEGER PRIMARY KEY,
  entity_id INTEGER NOT NULL, 
  position_x REAL, position_y REAL, position_z REAL, 
  rotation_x REAL, rotation_y REAL, rotation_z REAL, 
  scale_x REAL, scale_y REAL, scale_z REAL,
  FOREIGN KEY (entity_id) REFERENCES entities(entity_id) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS renderer(
  render_id INTEGER NOT NULL,
  entity_id INTEGER NOT NULL, 
  needs_light INTEGER NOT NULL,
  casts_shadow INTEGER NOT NULL,
  receives_shadows INTEGER NOT NULL,
  FOREIGN KEY (entity_id) REFERENCES entities(entity_id) ON DELETE CASCADE
  PRIMARY KEY (render_id)
);

CREATE TABLE IF NOT EXISTS textures_of_renderer(
  render_id INTEGER NOT NULL, 
  texture_id INTEGER NOT NULL,
  FOREIGN KEY (render_id) REFERENCES renderer(render_id) ON DELETE CASCADE, 
  FOREIGN KEY (texture_id) REFERENCES textures(texture_id) ON DELETE CASCADE,
  PRIMARY KEY (render_id, texture_id)
);

CREATE TABLE IF NOT EXISTS meshes_of_renderer(
  render_id INTEGER NOT NULL,
  mesh_id INTEGER NOT NULL,
  FOREIGN KEY (render_id) REFERENCES renderer(render_id) ON DELETE CASCADE, 
  FOREIGN KEY (mesh_id) REFERENCES meshes(mesh_id) ON DELETE CASCADE
  PRIMARY KEY (render_id, mesh_id)
);

CREATE TABLE IF NOT EXISTS camera(
  camera_id INTEGER PRIMARY KEY,
  entity_id INTEGER NOT NULL, 
  is_active INTEGER NOT NULL,
  mode INTEGER NOT NULL,
  pitch REAL NOT NULL,
  yaw REAL NOT NULL,
  roll REAL NOT NULL,
  position_x REAL NOT NULL, position_y REAL NOT NULL, position_z REAL NOT NULL,
  znear REAL NOT NULL,
  zfar REAL NOT NULL,
  left_ REAL, right_ REAL, bottom_ REAL, top_ REAL, 
  fov REAL, aspect_ratio REAL
);

CREATE TABLE IF NOT EXISTS tree(
  tree_id INTEGER PRIMARY KEY,
  entity_id INTEGER NOT NULL, 
  parent_id INTEGER NOT NULL,
  name TEXT NOT NULL
);