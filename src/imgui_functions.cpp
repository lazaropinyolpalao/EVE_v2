#include "imgui_functions.hpp"

#include <filesystem>
#ifdef RENDER_OPENGL
	#include "imgui.h"
	#include "imgui/imgui_impl_glfw.h"
	#include "imgui/imgui_impl_opengl3.h"
#endif
#ifdef RENDER_DIRECTX11
#include "imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#endif

#include <scene_manager.hpp>

static bool openDisplayWindowData = false;
static bool openDisplayTexture = false;

static bool openDisplayResourceList = false;
static bool openDisplaySceneGraph = false;
static bool openDisplayNewEntity = false;
static bool openDisplayEntityComponents = false;
static bool openDisplayAddTextureToRenderer = false;
static size_t selectedEntityComponent = 0;
static unsigned int selectedTexturePosition = -1;

static bool openDisplayMenuBar = true;

//Load new texture
static bool openDisplayLoadTexture = false;
static unsigned int selectedLoadTexture = 0;

//Load new cubemap texture
static bool openDisplayLoadCubemapTexture = false;

//Load new mesh
static bool openDisplayLoadMesh = false;
static unsigned int selectedLoadMesh = 0;

//Scenes
static bool openDisplayScenes = false;
static bool openDisplayDialogNewScene = false;
static bool openDisplayDialogSaveScene = false;
static bool openDisplayDialogLoadScene = false;

static unsigned int selectedEditScene = 0;
//static unsigned int selectedDeleteScene = 0;
static std::string selectedDeleteScene;
static unsigned int loadedScene = 0;
static char selectedEditedSceneAliasHolder[DATABASE_ALIAS_LENGTH] = {0};

//Audio
static int audio_selected = -1;
static std::string audio_name = "No audio file selected";

void ImguiFunctions::DisplayWindowData(Window* win, ComponentManager* comp){
	float fps_alpha = 0.9f;
	static double avgFps = 1.0;

	if (openDisplayWindowData) {
		ImGui::Begin("General info", &openDisplayWindowData);
		
		ImGui::Text("Screen size: %d x %d", win->width_, win->height_);
		ImGui::ColorEdit4("Background color ", &(win->clear_color_.r));

		float frames_this_second = (1.0f / ImGui::GetIO().DeltaTime);
		ImGui::Text("FPS: %d", (int)frames_this_second);

		avgFps = fps_alpha * avgFps + (1.0 - fps_alpha) * frames_this_second;
		ImGui::Text("Average framerate: %d", (int)avgFps);
		//ImGui::Text("Draw calls: %d", comp->draw_calls);

		ImGui::End();
	}
}

void ImguiFunctions::DisplayTexture(Texture* text){
	if (openDisplayTexture && text != nullptr) {
		ImGui::Begin(text->src_.c_str(), &openDisplayTexture, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
	#ifdef RENDER_OPENGL
		ImVec2 v = ImVec2((float)text->width_, (float)text->height_);
		ImU64 id = text->texture_id_;
		ImGui::Image((ImTextureID)id, v);
	#endif
	#ifdef RENDER_DIRECTX11
		ImGui::Image(((void*)text->shader_resource_view_.Get()), ImVec2((float)text->width_, (float)text->height_));
	#endif
		ImGui::End();
	}
}

void ImguiFunctions::DisplayLoadTexture(RenderSystem* render){
	std::string first = "";

	//Form vector of names
	std::vector<std::string> paths_;
#ifdef RENDER_OPENGL
	for (auto const& entry : std::filesystem::directory_iterator{ TEXTURES_FOLDER_PATH }){
		bool found = false;
		for (unsigned int i = 0; !found && i < render->resource_list_.textures_.size(); ++i) {
			if (entry.path().compare(render->resource_list_.textures_.at(i)->src_)==0) {
				found = true;
			}
		}

		if (!found) {
			paths_.push_back(entry.path().string());
		}
	}
#endif
	ImGui::Begin("Load new texture");
	if (paths_.size() != 0) {

		first = paths_[selectedLoadTexture];

		if (ImGui::BeginCombo("##ComboMeshes", first.c_str())) {
			for (unsigned int i = 0; i < paths_.size(); ++i) {

				if (ImGui::Selectable(paths_.at(i).c_str())) {
					selectedLoadTexture = i;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Confirm##ConfirmLoadTexture")) {
		#ifdef RENDER_OPENGL
			render->resource_list_.addTexture(paths_[selectedLoadTexture]);
		#endif
		#ifdef RENDER_DIRECTX11
			render->resource_list_.addTexture(static_cast<RenderSystemDirectX11*>(render)->getDevice(), paths_[selectedLoadTexture]);
		#endif
			openDisplayLoadTexture = false;
			selectedLoadTexture = 0;
		}
		ImGui::SameLine();
	}
	else{ImGui::Text("All textures are loaded");}
	if (ImGui::Button("Cancel##CancelLoadTexture")) {
		openDisplayLoadTexture = false;
		selectedLoadTexture = 0;
	}

	ImGui::End();
}

void ImguiFunctions::DisplayLoadMesh(RenderSystem* render){
	std::string first = "";

	//Form vector of names
	std::vector<std::string> paths_;
#ifdef RENDER_OPENGL
	for (auto const& entry : std::filesystem::directory_iterator{ MESHES_FOLDER_PATH }) {
		bool found = false;
		for (unsigned int i = 0; !found && i < render->resource_list_.meshes_.size(); ++i) {
			if (entry.path().compare(render->resource_list_.meshes_.at(i)->full_path_) == 0) {
				found = true;
			}
		}

		if (!found) {paths_.push_back(entry.path().string());}
	}
#endif

	ImGui::Begin("Load new mesh");
	if (paths_.size() != 0) {

		first = paths_[selectedLoadMesh];

		if (ImGui::BeginCombo("##ComboMeshes", first.c_str())) {
			for (unsigned int i = 0; i < paths_.size(); ++i) {

				if (ImGui::Selectable(paths_.at(i).c_str())) {
					selectedLoadMesh = i;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Confirm##ConfirmLoadMesh")) {
		#ifdef RENDER_OPENGL
			render->resource_list_.addMesh(paths_.at(selectedLoadMesh));
		#endif
		#ifdef RENDER_DIRECTX11
			RenderSystemDirectX11* r = static_cast<RenderSystemDirectX11*>(render);
			render->resource_list_.addMesh(r->getDevice(), r->getDeviceContext(), paths_.at(selectedLoadMesh));
		#endif
			openDisplayLoadMesh = false;
			selectedLoadMesh = 0;
		}
		ImGui::SameLine();
	}
	else { ImGui::Text("All meshes are loaded"); }
	if (ImGui::Button("Cancel##CancelLoadMesh")) {
		openDisplayLoadMesh = false;
		selectedLoadMesh = 0;
	}

	ImGui::End();
}

void ImguiFunctions::DisplayResourceList(ComponentManager* comp, RenderSystem* render) {
	static bool openDisplayResourceTreeNodeLighs = false;

	static Texture* selected_text= nullptr;
	static std::string cull_names[3] = { "FRONT", "BACK", "BOTH" };
	char str[50];

	if (openDisplayResourceList && comp != nullptr && render != nullptr) {

		ImGui::Begin("Resource List", &openDisplayResourceList);

		if (ImGui::TreeNode("Lights")) {

			ImGui::Text("Directional lights (Total: %d)", render->lights_.directional_.size());
			ImGui::SameLine();
			if (ImGui::Button("Add##AddDirectionalLight")) {render->lights_.addDirectionalLight();}

			for (unsigned int i = 0; i < render->lights_.directional_.size(); i++) {				
				int t = i+1;

				sprintf_s(str, "Directional Light %d", t);
				if (ImGui::TreeNodeEx(str, ImGuiTreeNodeFlags_AllowItemOverlap)) {
					sprintf_s(str, "Delete##DeleteDirectionalLight%d", t);
					if (ImGui::Button(str)) { 
						render->lights_.directional_.erase(render->lights_.directional_.begin() + i);
					}
					else{
						DirectionalLight* d = render->lights_.directional_[i].get();
						ImGui::SameLine();
						sprintf_s(str, "Visible##VisibleDir%d", t);
						if (ImGui::Checkbox(str, &d->visible_)) {d->changed_ = true;}
						if (d->visible_) {
							ImGui::Text("Pitch    ");  ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
							sprintf_s(str, "##PitchDir%d", t);
							if (ImGui::DragFloat(str, &d->pitch_, 1.0f, -180.0f, 180.0f, "%.2f")) {	d->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Yaw      ");  ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
							sprintf_s(str, "##YawDir%d", t);
							if (ImGui::DragFloat(str, &d->yaw_, 1.0f, -180.0f, 180.0f, "%.2f")) { d->changed_ = true; }

							ImGui::Text("Position");  ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##PositionDir%d", t);
							if (ImGui::DragFloat3(str, &d->position_.x, 0.5f)) { d->changed_ = true; }
							ImGui::Text("Direction");  ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##DirectionDir%d", t);
							if (ImGui::DragFloat3(str, &d->direction_.x, 0.01f, -1.0f, 1.0f, "%.2f")) { d->changed_ = true; }
							ImGui::Text("Ambient  ");  ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##AmbientDir%d", t);
							if (ImGui::DragFloat3(str, &d->ambient_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { d->changed_ = true; }
							ImGui::Text("Diffuse  ");  ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##DiffuseDir%d", t);
							if (ImGui::DragFloat3(str, &d->diffuse_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { d->changed_ = true; }
							ImGui::Text("Specular ");  ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##SpecularDir%d", t);
							if (ImGui::DragFloat3(str, &d->specular_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { d->changed_ = true; }

							ImGui::Text("Left  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##LeftDir%d", t);
							if(ImGui::DragFloat(str, &d->left_, 0.5f)){d->changed_ = true;}
							ImGui::SameLine();
							ImGui::Text("Right "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##RightDir%d", t);
							if (ImGui::DragFloat(str, &d->right_, 0.5f)) { d->changed_ = true; }
							ImGui::Text("Bottom"); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##BottomDir%d", t);
							if (ImGui::DragFloat(str, &d->bottom_, 0.5f)) { d->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Top   "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##TopDir%d", t);
							if (ImGui::DragFloat(str, &d->top_, 0.5f)) { d->changed_ = true; }
							ImGui::Text("Near  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##NearDir%d", t);
							if (ImGui::DragFloat(str, &d->znear_, 0.5f)) { d->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Far   "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##FarDir%d", t);
							if (ImGui::DragFloat(str, &d->zfar_, 0.5f)) { d->changed_ = true; }

						}
						if (d->changed_) {
							d->UpdateProjection();
							d->UpdateView();
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::Separator();

			ImGui::Text("Point lights (Total: %d)", render->lights_.point_.size());
			ImGui::SameLine();
			if (ImGui::Button("Add##AddPointLight")) { render->lights_.addPointLight(); }

			for (unsigned int i = 0; i < render->lights_.point_.size(); i++) {
				int t = i + 1;
				sprintf_s(str, "Point Light %d", t);
				if (ImGui::TreeNodeEx(str, ImGuiTreeNodeFlags_AllowItemOverlap)) {
					sprintf_s(str, "Delete##DeletePointLight%d", t);
					if(ImGui::Button(str)){ 
						render->lights_.point_.erase(render->lights_.point_.begin() + i);
					}
					else{
						PointLight* p = render->lights_.point_[i].get();
						ImGui::SameLine();
						sprintf_s(str, "Visible##VisiblePoint%d", t);
						if (ImGui::Checkbox(str, &p->visible_)) {p->changed_ = true;}
						if (p->visible_) {
							ImGui::Text("Position "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##PositionPoint%d", t);
							if (ImGui::DragFloat3(str, &p->position_.x, 0.1f)) { p->changed_ = true; }
							ImGui::Text("Ambient  "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##AmbientPoint%d", t);
							if (ImGui::DragFloat3(str, &p->ambient_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { p->changed_ = true; }
							ImGui::Text("Diffuse  "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##DiffusePoint%d", t);
							if (ImGui::DragFloat3(str, &p->diffuse_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { p->changed_ = true; }
							ImGui::Text("Specular "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##SpecularPoint%d", t);
							if (ImGui::DragFloat3(str, &p->specular_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { p->changed_ = true; }
							ImGui::Text("Constant "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##ConstantPoint%d", t);
							if (ImGui::DragFloat(str, &p->constant_, 0.01f, 0.0f, 1.0f, "%.2f")) { p->changed_ = true; }
							ImGui::Text("Linear   "); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##LinearPoint%d", t);
							if (ImGui::DragFloat(str, &p->linear_, 0.001f, 0.0f, 1.0f, "%.3f")) { p->changed_ = true; }
							ImGui::Text("Quadratic"); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##QuadraticPoint%d", t);
							if (ImGui::DragFloat(str, &p->quadratic_, 0.001f, 0.0f, 1.0f, "%.3f")) { p->changed_ = true; }


							ImGui::Text("FOV  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##FOVPoint%d", t);
							if (ImGui::DragFloat(str, &p->fov_, 0.5f, 20.0f, 120.0f, "%.2f")) { p->changed_ = true; }
							//ImGui::Text("Range  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							//sprintf_s(str, "##RangePoint%d", t);
							if (ImGui::DragFloat(str, &p->range_, 0.5f, 2.0f, 200.0f, "%.1f")) { p->changed_ = true; }
							ImGui::Text("Near  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##NearPoint%d", t);
							if (ImGui::DragFloat(str, &p->znear_, 0.5f)) { p->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Far   "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##FarPoint%d", t);
							if (ImGui::DragFloat(str, &p->zfar_, 0.5f)) { p->changed_ = true; }
						}


						if (p->changed_) {
							p->UpdateProjection();
							p->UpdateView();
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::Separator();

			ImGui::Text("Spot lights (Total: %d)", render->lights_.spot_.size());
			ImGui::SameLine();
			if (ImGui::Button("Add##AddSpotLight")) { render->lights_.addSpotLight(); }

			for (unsigned int i = 0; i < render->lights_.spot_.size(); i++) {

				int t = i + 1;
				sprintf_s(str, "Spot Light %d", t);
				if (ImGui::TreeNodeEx(str, ImGuiTreeNodeFlags_AllowItemOverlap)) {
					sprintf_s(str, "Delete##DeleteSpotLight%d", t);
					if (ImGui::Button(str)) { 
						render->lights_.spot_.erase(render->lights_.spot_.begin() + i);
					}
					else{
						SpotLight* s = render->lights_.spot_[i].get();
						ImGui::SameLine();
						sprintf_s(str, "Visible##VisibleSpot%d", t);
						if (ImGui::Checkbox("Visible", &s->visible_)) {s->changed_ = true;}
						if (s->visible_) {

							ImGui::Text("Pitch    ");  ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
							sprintf_s(str, "##PitchSpot%d", t);
							if (ImGui::DragFloat(str, &s->pitch_, 1.0f, -180.0f, 180.0f, "%.2f")) { s->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Yaw      ");  ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
							sprintf_s(str, "##YawSpot%d", t);
							if (ImGui::DragFloat(str, &s->yaw_, 1.0f, -180.0f, 180.0f, "%.2f")) { s->changed_ = true; }
					
							ImGui::Text("Position     ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##PositionSpot%d", t);
							if (ImGui::DragFloat3(str, &s->position_.x, 0.25f)) { s->changed_ = true; }
							ImGui::Text("Direction    ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##DirectionSpot%d", t);
							if (ImGui::DragFloat3(str, &s->direction_.x, 0.01f)) { s->changed_ = true; }
							ImGui::Text("Ambient      ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##AmbientSpot%d", t);
							if (ImGui::DragFloat3(str, &s->ambient_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { s->changed_ = true; }
							ImGui::Text("Diffuse      ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##DiffuseSpot%d", t);
							if (ImGui::DragFloat3(str, &s->diffuse_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { s->changed_ = true; }
							ImGui::Text("Specular     ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##SpecularSpot%d", t);
							if (ImGui::DragFloat3(str, &s->specular_.x, 0.01f, 0.0f, 1.0f, "%.2f")) { s->changed_ = true; }
							ImGui::Text("Cut Off      ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##Cut_OffSpot%d", t);
							if (ImGui::DragFloat(str, &s->cut_off_, 0.001f, s->outer_cut_off_, 1.0f, "%.3f")) { s->changed_ = true; }
							ImGui::Text("Outer Cut Off");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##Outer_Cut_OffSpot%d", t);
							if (ImGui::DragFloat(str, &s->outer_cut_off_, 0.001f, 0.0f, s->cut_off_, "%.3f")) { s->changed_ = true; }
							ImGui::Text("Constant     ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##ConstantSpot%d", t);
							if (ImGui::DragFloat(str, &s->constant_, 0.01f, 0.0f, 1.0f, "%.2f")) { s->changed_ = true; }
							ImGui::Text("Linear       ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##LinearSpot%d", t);
							if (ImGui::DragFloat(str, &s->linear_, 0.0001f, 0.0f, 1.0f, "%.4f")) { s->changed_ = true; }
							ImGui::Text("Quadratic    ");   ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f);
							sprintf_s(str, "##QuadraticSpot%d", t);
							if (ImGui::DragFloat(str, &s->quadratic_, 0.0001f, 0.0f, 1.0f, "%.4f")) { s->changed_ = true; }

							ImGui::Text("FOV  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##FOVSpot%d", t);
							if (ImGui::DragFloat(str, &s->fov_, 0.5f, 20.0f, 120.0f, "%.2f")) { s->changed_ = true; }
							//ImGui::Text("Range  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							//sprintf_s(str, "##RangeSpot%d", t);
							if (ImGui::DragFloat(str, &s->range_, 0.5f, 2.0f, 200.0f, "%.1f")) { s->changed_ = true; }
							ImGui::Text("Near  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##NearSpot%d", t);
							if (ImGui::DragFloat(str, &s->znear_, 0.5f)) { s->changed_ = true; }
							ImGui::SameLine();
							ImGui::Text("Far   "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
							sprintf_s(str, "##FarSpot%d", t);
							if (ImGui::DragFloat(str, &s->zfar_, 0.5f)) { s->changed_ = true; }
						}

						if (s->changed_) {
							s->UpdateProjection();
							s->UpdateView();
						}
					}

					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();

		}

		if (ImGui::TreeNode("Loaded textures")) {
			size_t num_text = render->resource_list_.textures_.size();
			ImGui::Text("Number of textures: %d", num_text);
			ImGui::SameLine();
			if (ImGui::Button("Add new##NewTexureButton")) { openDisplayLoadTexture = true; }

			for(size_t i=0;i<num_text; ++i){
				if (render->resource_list_.textures_[i]->loaded_) {
					if (ImGui::Button(render->resource_list_.textures_[i]->src_.c_str())) {
						openDisplayTexture = true;
						selected_text = render->resource_list_.textures_[i].get();
					}
				}
			}
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Loaded meshes")) {
			size_t num_meshes = render->resource_list_.meshes_.size();
			ImGui::Text("Number of meshes: %d", num_meshes);
			ImGui::SameLine();
			if (ImGui::Button("Add new##NewMeshButton")) {openDisplayLoadMesh = true;}

			if (ImGui::BeginTable("Meshes", 4)) {
				ImGui::TableSetupColumn("NAME");
				ImGui::TableSetupColumn("VERTICES");
				ImGui::TableSetupColumn("INDEXES");
				ImGui::TableSetupColumn("CULLING");
				ImGui::TableHeadersRow();


				for (size_t i = 0; i < num_meshes; ++i) {
					if (render->resource_list_.meshes_[i]->isInit_) {
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text(render->resource_list_.meshes_[i]->name_.c_str());
						ImGui::TableNextColumn();
						ImGui::Text("%zd", render->resource_list_.meshes_[i]->vertices_.size());
						ImGui::TableNextColumn();
						ImGui::Text("%zd", render->resource_list_.meshes_[i]->indexes_.size());
						ImGui::TableNextColumn();
						sprintf_s(str, "##MeshCullingType%d", (unsigned int)i);
						if (ImGui::BeginCombo(str, cull_names[render->resource_list_.meshes_[i]->cull_type_].c_str())) {
							//Hardcoded 3 because of cull_names size
							for (unsigned int j = 0; j < 3; ++j) {
								bool selected = (j == render->resource_list_.meshes_[i]->cull_type_);
								//If clicked, switch mesh with the new one
								if (ImGui::Selectable(cull_names[j].c_str(), selected)) {
									render->resource_list_.meshes_[i]->cull_type_ = j;
								}
								//if (selected)ImGui::SetItemDefaultFocus();
							}
							ImGui::End();
						}
					}
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Cubemap") && render->resource_list_.cubemap_!=nullptr) {
			Cubemap* cubemap = render->resource_list_.cubemap_.get();
			ImGui::Checkbox("Visible", &cubemap->visible_);
			char str[85] = "\0";
			sprintf_s(str, cubemap->textures_[cubemap->selected_texture_].get()->src_.c_str());
			ImGui::Text("Cubemap texture:");
			ImGui::SameLine();
			ImGui::Text(str);

			ImGui::Separator();

			ImGui::Text("Cubemap loaded textures:");
			ImGui::SameLine();
			if (ImGui::Button("Add new##NewCubemapTexureButton")) { openDisplayLoadCubemapTexture = true; }
			size_t num_text = cubemap->textures_.size();
			for (size_t i = 0; i < num_text; ++i) {
				if (cubemap->textures_[i]->loaded_) {
					if (ImGui::Button(cubemap->textures_[i]->src_.c_str())) {
						cubemap->selected_texture_ = (int) i;
						selected_text = cubemap->textures_[i].get();
					}
				}
			}

			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("On screen texts")) {
			if (ImGui::Button("Add new##AddNewTextButton")) {render->resource_list_.addTextToRender();}

			char text[TEXT_RENDERING_MAX_SIZE] = {'\n'};
			for (unsigned int i = 0; i < (unsigned int)render->resource_list_.screen_texts_.size(); i++) {

				if (render->resource_list_.screen_texts_.at(i) != nullptr) {

					ImGui::Text("Text number: %d", i);

					RenderingText* r = render->resource_list_.screen_texts_.at(i).get();

					sprintf_s(str, "##TextRender%d", (unsigned int)i);
					strcpy_s(text, r->text_.c_str());
					if (ImGui::InputText(str, text, TEXT_RENDERING_MAX_SIZE)) {
						r->text_.assign(text);
					}

					sprintf_s(str, "##TextRenderPosX%d", (unsigned int)i);
					ImGui::SetNextItemWidth(50.0f);
					ImGui::Text("X:"); ImGui::SameLine();
					ImGui::SetNextItemWidth(50.0f);
					ImGui::DragFloat(str, &r->pos_x); ImGui::SameLine();

					sprintf_s(str, "##TextRenderPosY%d", (unsigned int)i);
					ImGui::SetNextItemWidth(50.0f);
					ImGui::Text("Y:"); ImGui::SameLine();
					ImGui::SetNextItemWidth(50.0f);
					ImGui::DragFloat(str, &r->pos_y); ImGui::SameLine();

					sprintf_s(str, "##TextRenderPosScale%d", (unsigned int)i);
					ImGui::SetNextItemWidth(50.0f);
					ImGui::Text("Scale:"); ImGui::SameLine();
					ImGui::SetNextItemWidth(50.0f);
					ImGui::DragFloat(str, &r->scale_);

					sprintf_s(str, "##TextRenderColor%d", (unsigned int)i);
					ImGui::SetNextItemWidth(150.0f);
					ImGui::ColorEdit3(str, &r->color_.x);

					ImGui::Text("Actions:");
					sprintf_s(str, "Delete##TextRenderDelete%d", (unsigned int)i);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::Button(str)) {
						if (i == 0) {
							render->resource_list_.screen_texts_.erase(render->resource_list_.screen_texts_.begin());
						}
						else {
							render->resource_list_.screen_texts_.erase(render->resource_list_.screen_texts_.begin() + i);
						}
					}
					ImGui::SameLine();
					sprintf_s(str, "Duplicate##TextRenderDuplicate%d", (unsigned int)i);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::Button(str)) {
						render->resource_list_.addTextToRender(r->text_.c_str(), r->pos_x, r->pos_y, r->color_, r->scale_);
					}

					//Separator only if not last element
					if (i + 1 < (unsigned int)render->resource_list_.screen_texts_.size()) { ImGui::Separator(); }
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Audio")) {

			if (ImGui::BeginCombo("##AudioSelected", audio_name.c_str())) {
				for (unsigned int i = 0; i < render->resource_list_.audios_.size(); i++) {
					Audio* audio = render->resource_list_.audios_.at(i).get();
					if (ImGui::Selectable(audio->filepath_.c_str())) {
						audio_selected = i;
						audio_name = audio->filepath_;
					}
				}
				ImGui::EndCombo();
			}

			for (unsigned int i = 0; i < render->resource_list_.audios_.size(); i++) {

				if (i == audio_selected) {
					Audio* audio = render->resource_list_.audios_.at(i).get();

					float vlm = audio->gain_;
					ImGui::SliderFloat("Audio Volume", &vlm, 0.0f, 1.0f, "%.2f");
					audio->setGain(vlm);

					bool loop = audio->looping();
					ImGui::Checkbox("Loop", &loop);
					audio->setLooping(loop);

					if (ImGui::Button("Play")) {
						for (unsigned int i = 0; i < render->resource_list_.audios_.size(); i++) {
							if (render->resource_list_.audios_.at(i).get() != audio) {
								render->resource_list_.audios_.at(i).get()->stop();
							}
						}
						audio->play();
					}

					if (ImGui::Button("Pause")) {
						audio->pause();
					}

					if (ImGui::Button("Stop")) {
						audio->stop();
					}
				}

			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	if (openDisplayTexture && selected_text!=nullptr) { DisplayTexture(selected_text); }
	if (openDisplayLoadMesh) { DisplayLoadMesh(render); }
	if (openDisplayLoadTexture) { DisplayLoadTexture(render); }
}

void ImguiFunctions::DisplaySceneGraph(ComponentManager* comp){
	static size_t tree_hash = typeid(TreeComponent).hash_code();
	if (openDisplaySceneGraph && comp != nullptr) {
		ImGui::Begin("Scene Graph", &openDisplaySceneGraph);

		if (ImGui::Button("New Entity")) {
			openDisplayNewEntity = true;
		}

		std::vector<component_node<TreeComponent>>* tree_comps_ = 
			&(*static_cast<component_list<TreeComponent>*>(comp->components_classes_.find(tree_hash)->second.get())).components_;

		for (unsigned int i = 0; i < comp->scene_tree_roots_.size(); ++i) {
			DrawTree(tree_comps_, comp->scene_tree_roots_.at(i));
		}

		ImGui::End();

		if (openDisplayNewEntity) {
			DisplayNewEntity(comp);
		}
	}
}

void ImguiFunctions::DisplayNewEntity(ComponentManager* comp){

	//One variable for each component
	static bool selectedTransform = false;
	static bool selectedRenderer = false;
	static bool selectedCamera = false;

	//static char entity_name[ENTITY_NAME_LENGTH];

	if (ImGui::Begin("Create new entity", &openDisplayNewEntity)) {
		ImGui::Text("Select the components you want to add");
		ImGui::Checkbox("Transform", &selectedTransform);
		ImGui::Checkbox("Renderer", &selectedRenderer);
		ImGui::Checkbox("Camera", &selectedCamera);
		//ImGui::InputText("##NewEntityName", entity_name, ENTITY_NAME_LENGTH);
		ImGui::Separator();
		if (ImGui::Button("Create##CreateNewEntity")) {
			
			size_t id = comp->new_entity();
			if (selectedTransform) {comp->addComponent<TransformComponent>(id);}
			if (selectedRenderer) {comp->addComponent<RendererComponent>(id);}
			if (selectedCamera) {comp->addComponent<CameraComponent>(id);}

			openDisplayNewEntity = false;
			selectedTransform = false;
			selectedRenderer = false;
			selectedCamera = false;
		}
		ImGui::SameLine();
		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelNewEntity")) {
			openDisplayNewEntity = false;
			selectedTransform = false;
			selectedRenderer = false;
			selectedCamera = false;
		}
		ImGui::End();
	}
}

void ImguiFunctions::DrawTree(std::vector<component_node<TreeComponent>>* tree_comps_, size_t entity) {
	//Get the component first
	TreeComponent *t = nullptr;
	for (unsigned int i = 0; t== nullptr && i < tree_comps_->size(); ++i) {
		if (tree_comps_->at(i).entity_id_ == entity) {t= &(tree_comps_->at(i).data_);}
	}

	if (t != nullptr) {
		std::string temp_s = t->name;
		if (temp_s.empty()) {
			temp_s = "Unknown";
		}

		temp_s.append("##");
		temp_s.append(std::to_string(entity));

		//sprintf_s(str, "Entity %zd", entity);
		if (t->num_children_ == 0) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
			flags = (selectedEntityComponent == entity? flags | ImGuiTreeNodeFlags_Selected: flags);
			if (ImGui::TreeNodeEx(temp_s.c_str(), flags)) {
				if (ImGui::IsItemClicked(0)) {
					selectedEntityComponent = entity;
					openDisplayEntityComponents = true;
				}
				ImGui::TreePop();
			}
		}
		else {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			flags = (selectedEntityComponent == entity ? flags | ImGuiTreeNodeFlags_Selected : flags);
			bool temp = ImGui::TreeNodeEx(temp_s.c_str(), flags);
			if (ImGui::IsItemClicked() && 
					(ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing()){
				selectedEntityComponent = entity;
				openDisplayEntityComponents = true;
			}
			if (temp) {
				for (unsigned int i = 0; i < MAX_TREE_CHILDREN; ++i) {
					if (t->children_[i] != 0) { DrawTree(tree_comps_, t->children_[i]); }
				}
				ImGui::TreePop();
			}
		}


		
	}
}

void ImguiFunctions::DisplayEntityComponents(ComponentManager* comp, RenderSystem* renderer) {

	static size_t transform_hash = typeid(TransformComponent).hash_code();
	static size_t render_hash = typeid(RendererComponent).hash_code();
	static size_t camera_hash = typeid(CameraComponent).hash_code();
	static size_t tree_hash = typeid(TreeComponent).hash_code();
	static std::string camera_modes[2] = {"PERSPECTIVE", "ORTHOGRAPHIC"};

	if (openDisplayEntityComponents && selectedEntityComponent != 0 && nullptr != comp) {
		//Find each list of the resources to find the components of the selected Entity
		std::vector<component_node<TransformComponent>>* transform_vec = &(*static_cast<component_list<TransformComponent>*>(comp->components_classes_.find(transform_hash)->second.get())).components_;
		size_t transform_size = transform_vec->size();
		std::vector<component_node<RendererComponent>>* render_vec = &(*static_cast<component_list<RendererComponent>*>(comp->components_classes_.find(render_hash)->second.get())).components_;
		size_t render_size = render_vec->size();
		std::vector<component_node<CameraComponent>>* camera_vec = &(*static_cast<component_list<CameraComponent>*>(comp->components_classes_.find(camera_hash)->second.get())).components_;
		size_t camera_size = camera_vec->size();
		std::vector<component_node<TreeComponent>>* tree_vec = &(*static_cast<component_list<TreeComponent>*>(comp->components_classes_.find(tree_hash)->second.get())).components_;
		size_t tree_size = tree_vec->size();
		
		size_t max_size = transform_vec->size();
		if(render_vec->size()>max_size){max_size = render_vec->size();}
		if(camera_vec->size()>max_size){max_size = camera_vec->size();}
		if(tree_vec->size()>max_size){max_size = tree_vec->size();}

		TransformComponent* transform = nullptr;
		RendererComponent* render = nullptr;
		CameraComponent* camera = nullptr;
		TreeComponent* tree = nullptr;

		bool allFound = false;
		for (size_t i = 0; !allFound && i < max_size; ++i) {
			if (i < transform_size && transform == nullptr && transform_vec->at(i).entity_id_ == selectedEntityComponent){
					transform = &(transform_vec->at(i).data_);
			}

			if (i < render_size && render == nullptr && render_vec->at(i).entity_id_ == selectedEntityComponent) {
					render = &(render_vec->at(i).data_);
			}

			if (i < camera_size && camera == nullptr && camera_vec->at(i).entity_id_ == selectedEntityComponent) {
				camera = &(camera_vec->at(i).data_);
			}

			if (i < tree_size && tree == nullptr && tree_vec->at(i).entity_id_ == selectedEntityComponent) {
				tree = &(tree_vec->at(i).data_);
			}

			allFound = (nullptr!= transform && nullptr != render && nullptr != camera && nullptr != tree);
		}
		

		ImGui::Begin("Component viewer", &openDisplayEntityComponents);
		char str[50];
		sprintf_s(str, "Entity id: %zd", selectedEntityComponent);
		if (tree != nullptr) {

			if (strlen(tree->name)==0) {
				ImGui::Text("Unknown");
			}
			else {
				ImGui::Text(tree->name);
			}
			ImGui::Text(str);
		}
		

		if (ImGui::Button("Delete entity")) {
			comp->remove_entity(selectedEntityComponent);
			openDisplayEntityComponents = false;
			selectedEntityComponent = 0;
		}

		//Transform component
		if(nullptr != transform && ImGui::TreeNode("Transform component")) {
			ImGui::Text("Position"); ImGui::SameLine(); 
			if (ImGui::DragFloat3("##PositionTransform", &transform->position_.x)) {
				transform->changed_ = true;
			}
			ImGui::Text("Rotation"); ImGui::SameLine(); 
			if(ImGui::DragFloat3("##RotationTransform", &transform->rotation_.x)) {
				transform->changed_ = true;
			}
			ImGui::Text("Scale   "); ImGui::SameLine(); 
			if(ImGui::DragFloat3("##ScaleTransform", &transform->scale_.x)) {
				transform->changed_ = true;
			}
			transform->GetMatrix();
			
			ImGui::TreePop();
		} 
		
		//Render component
		if (nullptr != render && ImGui::TreeNode("Render component")) {
			if(render->isInit_){
				sprintf_s(str, "Mesh name '%s'", render->mesh_->name_.c_str());
				ImGui::Text(str);
			}
			else{ ImGui::Text("No mesh"); }
			ImGui::Checkbox("Needs light", &render->needs_light_);
			ImGui::Checkbox("Casts shadows", &render->casts_shadows_);
			ImGui::Checkbox("Receive shadows", &render->receives_shadows_);
			ImGui::Separator();

			if (ImGui::BeginCombo("##RenderMeshSelected", (render->isInit_?render->mesh_.get()->name_.c_str():"No mesh selected"))) {
				for (unsigned int i = 0; i < renderer->resource_list_.meshes_.size(); ++i) {
					bool selected = (render->isInit_ && renderer->resource_list_.meshes_.at(i).get() == render->mesh_.get());
					//If clicked, switch mesh with the new one
					if (ImGui::Selectable(renderer->resource_list_.meshes_.at(i).get()->name_.c_str(), selected)) {
						render->ChangeMesh(renderer->resource_list_.meshes_.at(i));
					}
					if (selected)ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();
			ImGui::Text("Textures"); ImGui::SameLine();
			if (ImGui::Button("Add texture")) {openDisplayAddTextureToRenderer = true;}
			std::vector<std::shared_ptr<Texture>>::iterator it = render->textures_.begin();
			char str[50];
			if (render->textures_.size() == 0) {ImGui::Text("No textures added");}
			else {
				for(unsigned int i = 0; i < render->textures_.size(); ++i){
					ImGui::Text(render->textures_.at(i)->src_.c_str());
					ImGui::SameLine();
					sprintf_s(str, "Remove##RemoveTexture%d", i);
					if (ImGui::Button(str)) {
						render->textures_.erase(it + i);
					}
				}
			}

			if (openDisplayAddTextureToRenderer) {
				DisplayAddTextureToRenderer(comp, render, renderer);
			}

			ImGui::TreePop();
		}
		//Camera component
		if (nullptr != camera && ImGui::TreeNode("Camera component")) {
			bool change = false;
			if (ImGui::BeginCombo("##ModeCamera", camera_modes[(unsigned int)camera->mode_].c_str())) {
				//Hardcoded 2 because of camera_modes size
				for (unsigned int j = 0; j < 2; ++j) {
					bool selected = (j == (unsigned int)camera->mode_);
					//If clicked, switch camera mode
					if (ImGui::Selectable(camera_modes[j].c_str(), selected)) {
						camera->mode_ = (CameraMode)j;
						change = true;
					}
				}
				ImGui::End();
			}
			ImGui::Text("Position"); ImGui::SameLine();
			if (ImGui::DragFloat3("##PositionCamera", &camera->position_.x)) { change = true; }
			ImGui::Text("Pitch/Yaw"); ImGui::SameLine();
			if (ImGui::DragFloat2("##RotationCamera", &camera->pitch_)) { change = true; }
			ImGui::Text("Near/Far"); ImGui::SameLine();
			if (ImGui::DragFloat2("##NearFarCamera", &camera->znear_)) { change = true; }
			
			ImGui::Text("Ortographic values"); ImGui::SameLine(); ImGui::Separator();
			ImGui::Text("Left/Right"); ImGui::SameLine();
			if (ImGui::DragFloat2("##LeftRightCamera", &camera->left_)) { change = true; }
			ImGui::Text("Bottom/Top"); ImGui::SameLine();
			if (ImGui::DragFloat2("##BottomTopCamera", &camera->bottom_)) { change = true; }

			ImGui::Text("Perspective values"); ImGui::SameLine(); ImGui::Separator();
			ImGui::Text("FOV  "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
			if(ImGui::DragFloat("##FOVCamera", &camera->fov_, 0.5f, 20.0f, 120.0f, "%.2f")){change = true;}

			ImGui::Separator();
			ImGui::Text("Camera Speed");
			if (ImGui::DragFloat("##SpeedCamera", &camera->speed_, 0.1f, 0.1f, 5.0f, "%.2f")) { change = true; }
			
			if (change) {
				if(camera->mode_ == CameraMode::kOrthographic){
					camera->SetOrthographic(camera->left_, camera->right_, camera->bottom_, camera->top_, camera->znear_, camera->zfar_);
				}
				else {
					camera->SetPerspective(camera->fov_, camera->aspect_ratio_, camera->znear_, camera->zfar_);
				}
				//Updat the camera without moving it
				camera->UpdateCamera({}, 0.0f);
			}

			ImGui::TreePop();
		}

		//Tree component
		if (nullptr != tree && ImGui::TreeNode("Tree component")) {

			char text[TEXT_RENDERING_MAX_SIZE] = { '\n' };

			ImGui::Text("Name: "); ImGui::SameLine();
			sprintf_s(str, "##TextRender%d", (unsigned int)selectedEntityComponent);
			ImGui::InputText(str, tree->name, TEXT_RENDERING_MAX_SIZE);

			ImGui::Text("Number of children %d", tree->num_children_);
			if (comp->num_entities_ > 1) {

				ImGui::Text("Parent entity: "); ImGui::SameLine();
				if (tree->parent_ != 0) {sprintf_s(str, "%zd", tree->parent_);}
				else{ sprintf_s(str, "%s", "No parent selected"); }
				
				if (tree->parent_ != 0 && ImGui::Button("Remove parent")) {
					comp->remove_parent(selectedEntityComponent);
				}
				//*/
				if (ImGui::BeginCombo("##SetNewParentCombo", str)) {
					for (size_t i = 1; i <= comp->num_entities_; i++) {
						std::vector<size_t>::iterator found = std::find(comp->deleted_entities_.begin(), comp->deleted_entities_.end(), i);
						if (i != selectedEntityComponent && !comp->IsMyChild(selectedEntityComponent, i) && found == comp->deleted_entities_.end()) {
							sprintf_s(str, "Entity %zd", i);
							if (ImGui::Selectable(str, (i == selectedEntityComponent))) {
								comp->make_parent(i, selectedEntityComponent);
							}
						}
					}
					ImGui::EndCombo();
				}
				/**/
			}
			else {ImGui::Text("Parent entity: No other entities");}

			ImGui::TreePop();
		}

		ImGui::End();
	}
}

void ImguiFunctions::DisplayAddTextureToRenderer(ComponentManager* comp, RendererComponent* renderer, RenderSystem* render_sys){

	if (ImGui::Begin("Add new texture", &openDisplayAddTextureToRenderer)) {
		
		//Get first element that can be added
		std::string first = "";
		std::vector<std::shared_ptr<Texture>>::iterator s, e;
		s = renderer->textures_.begin();
		e = renderer->textures_.end();
		size_t size = render_sys->resource_list_.textures_.size();
		if (selectedTexturePosition == -1) {
			//Find the first texture that it's not already in the renderer
			for (unsigned int i = 0; first.empty() && i < size; ++i) {
				bool found = false;
				for (unsigned int j = 0; !found && j < renderer->textures_.size(); ++j) {
					#ifdef RENDER_OPENGL
					if (render_sys->resource_list_.textures_.at(i)->texture_id_ == renderer->textures_.at(j)->texture_id_) {
						found = true;
					}
					#endif
					#ifdef RENDER_DIRECTX11
					if (render_sys->resource_list_.textures_.at(i)->texture_ == renderer->textures_.at(j)->texture_) {
						found = true;
					}
					#endif
				}

				if (!found) {
					first = render_sys->resource_list_.textures_.at(i)->src_;
					selectedTexturePosition = i;
				}
			}
		}
		else {first = render_sys->resource_list_.textures_.at(selectedTexturePosition)->src_;}

		if (size > renderer->textures_.size()) {

			ImGui::Text("Select the texture to add:");
			if (ImGui::BeginCombo("##texurecombo", first.c_str())) {
				for (unsigned int i = 0; i < size; ++i) {
					//Display only if it doesn't have it already
					bool found = false;
					for (unsigned int j = 0; !found && j < renderer->textures_.size(); ++j) {
						#ifdef RENDER_OPENGL
						if (render_sys->resource_list_.textures_.at(i)->texture_id_ == renderer->textures_.at(j)->texture_id_) {
							found = true;
						}
						#endif
						#ifdef RENDER_DIRECTX11
						if (render_sys->resource_list_.textures_.at(i)->texture_ == renderer->textures_.at(j)->texture_) {
							found = true;
						}
						#endif
					}

					if (!found && ImGui::Selectable(render_sys->resource_list_.textures_.at(i)->src_.c_str())) {
						selectedTexturePosition = i;
					}
				}
				ImGui::EndCombo();
			}
		}
		else {ImGui::Text("All textures added");}


		if (selectedTexturePosition!=-1) {
			if (ImGui::Button("Add selected texture##ConfirmNewTexture")) {

				renderer->textures_.push_back(render_sys->resource_list_.textures_.at(selectedTexturePosition));

				selectedTexturePosition = -1;
				openDisplayAddTextureToRenderer = false;
			}
			ImGui::SameLine();
		}

		if (ImGui::Button("Cancel##CancelNewTexture")) {
			selectedTexturePosition = -1;
			openDisplayAddTextureToRenderer = false;
		}

		ImGui::End();
	}
}

void ImguiFunctions::DisplayScenes(SceneManager* sm) {

	static ImU32 row_bg_color_blue = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.25f, 0.65f));
	static ImU32 row_bg_color_red = ImGui::GetColorU32(ImVec4(0.25f, 0.0f, 0.0f, 0.65f));

	if (openDisplayScenes) {
		ImGui::Begin("Saved scenes", &openDisplayScenes);
		//SceneManager* s = e->getSceneManager();
		if (ImGui::BeginTable("Scenes", 3)) {

			char str[50];
			ImGui::TableSetupColumn("ALIAS");
			ImGui::TableSetupColumn("LAST UPDATE");
			ImGui::TableSetupColumn("ACTIONS");
			ImGui::TableHeadersRow();
			
			ImGui::TableHeader("##SceneTableHeader");
			SceneData temp_s;
			std::string temp;
			for(unsigned int i=0;i< sm->scenes_.size();++i){
				temp_s = sm->scenes_.at(i);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				temp = temp_s.alias;
				ImGui::Text("%s", temp.substr(0, temp.rfind(".db")).c_str());
				ImGui::TableNextColumn();
				temp = std::format("{}", temp_s.last_update);
				ImGui::Text("%s", temp.substr(0, temp.rfind(".")).c_str());
				ImGui::TableNextColumn();
				
				/*
					sprintf_s(str, "Edit##SceneDetailsEdit%d", s->scenes_.at(i).scene_id_);
					if (ImGui::Button(str)) {
						strncpy_s(selectedEditedSceneAliasHolder, s->scenes_.at(i).alias, DATABASE_ALIAS_LENGTH);
						selectedEditScene = s->scenes_.at(i).scene_id_;
						selectedDeleteScene = 0;
					}
					ImGui::SameLine();
				*/
				sprintf_s(str, "Delete##SceneDetailsDelete-%s", temp_s.alias);
				if (ImGui::Button(str)) {
					selectedDeleteScene = temp_s.alias;
					//selectedEditScene = 0;
				}

				/*
				if (s->scenes_.at(i).scene_id_ == selectedEditScene) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + 1, row_bg_color_blue);
				}
				*/
				//if (s->scenes_.at(i).scene_id_ == selectedDeleteScene) {
				if (selectedDeleteScene.compare(temp_s.alias) == 0) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + 1, row_bg_color_red);
				}
			}

			ImGui::EndTable();
		}
		ImGui::End();

		/*
		if (selectedEditScene != 0) {
			DisplayEditSceneDetails(e, selectedEditScene);
		}
		else 
		*/
		if (!selectedDeleteScene.empty()) {DisplayDeleteScene(sm);}
	}
}

/*
void ImguiFunctions::DisplayEditSceneDetails(Engine* e, unsigned int scene_id){

	SceneData data;
	SceneManager *s = e->getSceneManager();
	bool found = false;
	for (unsigned int i = 0; !found && i < s->scenes_.size(); ++i) {
		if (s->scenes_[i].scene_id_ == scene_id) {
			data = s->scenes_[i];
			found = true;
		}
	}

	if (found && ImGui::Begin("Edit scene details")) {

		ImGui::Text("Scene id:    %d", data.scene_id_);
		ImGui::Text("Alias:       %s", data.alias);
		ImGui::Text("Created at:  %s", data.created);
		ImGui::Text("Last update: %s", data.last_update);
		ImGui::Separator();
		ImGui::Text("New alias:");
		ImGui::InputText("##newAlias", selectedEditedSceneAliasHolder, DATABASE_ALIAS_LENGTH);
		ImGui::Separator();

		//If change is confirmed, update the database with the scene and the new alias, and reload the scene list
		if (strlen(selectedEditedSceneAliasHolder)!=0 && ImGui::Button("Confirm##ConfirmEditSceneEdit")) {
			s->UpdateSceneAlias(selectedEditedSceneAliasHolder, data.scene_id_);
			selectedEditScene = 0;

		}
		ImGui::SameLine();
		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelEditSceneEdit")) {
			selectedEditScene = 0;
		}
		

		ImGui::End();
	}
}
*/

void ImguiFunctions::DisplayDeleteScene(SceneManager* s) {

	SceneData data;
	//SceneManager* s = e->getSceneManager();
	bool found = false;
	for (unsigned int i = 0; !found && i < s->scenes_.size(); ++i) {
		if (selectedDeleteScene.compare(s->scenes_[i].alias) == 0) {
			data = s->scenes_[i];
			found = true;
		}
	}

	if (found && ImGui::Begin("Delete scene")) {
		std::string temp = data.alias;
		ImGui::Text("Alias:       %s", temp.substr(0, temp.rfind(".db")).c_str());
		ImGui::Text("Path:        %s", data.path);

		temp = std::format("{}", data.last_update);
		ImGui::Text("Last update: %s", temp.substr(0, temp.rfind(".")).c_str());
		ImGui::Separator();
		ImGui::Text("Are you sure you want to DELETE this scene? This is not undoable");

		//If confirmed, delete the selected scene
		if (ImGui::Button("Confirm##ConfirmDeleteScene")) {
			s->DeleteScene(data);
			selectedDeleteScene = "";

		}
		ImGui::SameLine();
		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelDeleteScene")) {
			selectedDeleteScene = "";
		}


		ImGui::End();
	}

}

void ImguiFunctions::DisplayNewScene(ComponentManager* comp, RenderSystem* rs) {

	if(ImGui::Begin("New scene")) {
		ImGui::Text("Open new scene? All changes not saved will be lost");

		//If confirmed, clear scene and set to new
		if (ImGui::Button("Confirm##ConfirmNewScene")) {
			ResetImguiMenus();
			comp->ResetComponentSystem();

#ifdef RENDER_OPENGL
			static_cast<RenderSystemOpenGL*>(rs)->ResetResources();
#endif
		}
		ImGui::SameLine();
		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelNewScene")) { 
			openDisplayDialogNewScene = false; 
		}


		ImGui::End();
	}
}

void ImguiFunctions::DisplaySaveNewScene(ComponentManager* cm, RenderSystem* rs, SceneManager* sm) {
	static char new_alias[DATABASE_ALIAS_LENGTH];
	static ImVec4 original_button_color = ImGui::GetStyle().Colors[ImGuiCol_Button];
	static ImVec4 original_button_color_active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
	static ImVec4 original_button_color_hover = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];


	static ImVec4 overwrite_button_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	if (ImGui::Begin("Save new scene", &openDisplayDialogSaveScene)) {

		ImGui::Text("Input the alias for the new scene");
		ImGui::InputText("##SaveNewSceneAlias", new_alias, DATABASE_ALIAS_LENGTH);

		//Display confirm only if not empty
		if (strnlen_s(new_alias, DATABASE_ALIAS_LENGTH) != 0) {

			bool exists = SceneManager::ExistsDB(new_alias);
			//Show confirm if not exists already, else, show "overwrite"
			if (!exists) {
				if (ImGui::Button("Confirm##ConfirmSaveNewScene")) {
					int result = sm->SaveNewScene(cm, rs, std::string(new_alias));
					//If worked
					if (result != -1) {
						//loadedScene = result;
					#ifdef RENDER_OPENGL
						static_cast<RenderSystemOpenGL *>(rs)->getWindow()->set_title(new_alias);
					#endif
					}
					ResetImguiMenus();
					memset(new_alias, '\0', DATABASE_ALIAS_LENGTH);
				}
			}
			else{
				
				ImGui::GetStyle().Colors[ImGuiCol_Button] = overwrite_button_color;
				ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = overwrite_button_color;
				ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = overwrite_button_color;
				if (ImGui::Button("OverWrite##ConfirmOverwriteScene")) {

					ResetImguiMenus();
					memset(new_alias, '\0', DATABASE_ALIAS_LENGTH);
				}

				ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = original_button_color_active;
				ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = original_button_color_hover;
				ImGui::GetStyle().Colors[ImGuiCol_Button] = original_button_color;
			}
			ImGui::SameLine();
		}


		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelSaveNewScene")) {
			openDisplayDialogSaveScene = false;
			memset(new_alias, '\0', DATABASE_ALIAS_LENGTH);
		}

		ImGui::End();
	}
}

void ImguiFunctions::DisplayLoadScene(ComponentManager* cm, RenderSystem* rs, SceneManager* sm) {
	static unsigned int selectedLoadScene = 0;
	if (ImGui::Begin("Load scene")) {
		ImGui::Text("Load a different scene? All changes not saved will be lost");

		if (sm->scenes_.size() != 0) {

			char str[512];
			//sprintf_s(str, "%s - Last update(%s)", e->getSceneManager()->scenes_.at(selectedLoadScene).alias, std::format("{}", e->getSceneManager()->scenes_.at(selectedLoadScene).last_update).c_str());

			std::string temp_str = sm->scenes_.at(selectedLoadScene).alias;
			sprintf_s(str, "%s", temp_str.substr(0, temp_str.rfind(".")).c_str());
			if (ImGui::BeginCombo("##SelectLoadScene", str)) {
			//Display list of scenes
				for (unsigned int i = 0; i < sm->scenes_.size(); ++i) {
					SceneData sd = sm->scenes_.at(i);
					temp_str = sd.alias;
					sprintf_s(str, "%s", temp_str.substr(0, temp_str.rfind(".")).c_str());
					if (ImGui::Selectable(str, i==selectedLoadScene)) {
						selectedLoadScene = i;
					}
				}
				ImGui::EndCombo();
			}
	

			//If confirmed, clear scene and set to new
			if (ImGui::Button("Confirm##ConfirmLoadScene")) {
				ResetImguiMenus();
				std::string temp_str = sm->scenes_.at(selectedLoadScene).alias;
				sm->LoadScene(cm, rs, temp_str.substr(0, temp_str.rfind(".")).c_str());

				//Load given scene
				openDisplayDialogLoadScene = false;
			}
			ImGui::SameLine();
		}


		//Just close and bye bye
		if (ImGui::Button("Cancel##CancelLoadScene")) {
			openDisplayDialogLoadScene = false;
		}


		ImGui::End();
	}
}

void ImguiFunctions::DisplayMenuBar(Window* win, SceneManager* scene_manager){

	if (openDisplayMenuBar) {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Main Menu", openDisplayMenuBar)) {
				if (ImGui::MenuItem("Display Window Data")) {openDisplayWindowData = true;}
				if (ImGui::MenuItem("Quit", "Escape")) {win->close_window();}
				ImGui::EndMenu();
			}
			
#ifdef RENDER_OPENGL
			if (ImGui::BeginMenu("Scene")) {
				if (ImGui::MenuItem("Display Scenes List")) {
					openDisplayScenes = true;
				}
				if (ImGui::MenuItem("New Scene")) {
					openDisplayDialogNewScene = true;

					//Close the other menus
					openDisplayDialogSaveScene = false;
					openDisplayDialogLoadScene = false;
				}
				if (ImGui::MenuItem("Save Scene")) {
					//Open different menu if the actual scene is not previously saved
					if (loadedScene == 0) {
						openDisplayDialogSaveScene = true;
					}
					//Save scene directly
					else {
					}

					//Close the other menus
					openDisplayDialogNewScene = false;
					openDisplayDialogLoadScene = false;
				}
				if (ImGui::MenuItem("Load Scene")) {
					//Reload scenes
					scene_manager->QueryScenes();
					openDisplayDialogLoadScene = true;

					//Close the other menus
					openDisplayDialogNewScene = false;
					openDisplayDialogSaveScene = false;
				}
				ImGui::EndMenu();
			}
#endif

			if (ImGui::Button("Scene Graph")) {openDisplaySceneGraph = true;}
			if (ImGui::Button("Display Resource List")) { openDisplayResourceList = true; }
			ImGui::EndMainMenuBar();
		}
	}

	//Shortcut management
	if(win->return_last_input().escape){win->close_window();}
	
}

void ImguiFunctions::ResetImguiMenus(){
	openDisplayWindowData = false;
	openDisplayTexture = false;

	openDisplayResourceList = false;
	openDisplaySceneGraph = false;
	openDisplayNewEntity = false;
	openDisplayEntityComponents = false;
	openDisplayAddTextureToRenderer = false;
	selectedEntityComponent = 0;
	selectedTexturePosition = -1;

	openDisplayMenuBar = true;

	openDisplayLoadTexture = false;
	selectedLoadTexture = 0;

	openDisplayLoadMesh = false;
	selectedLoadMesh = 0;

	openDisplayScenes = false;
	openDisplayDialogNewScene = false;
	openDisplayDialogSaveScene = false;
	openDisplayDialogLoadScene = false;

	//selectedEditScene = 0;
	selectedDeleteScene = "";
	loadedScene = 0;
}

void ImguiFunctions::ImguiFunctionCalls(ComponentManager* cm, RenderSystem* rs, SceneManager* sm) {

#ifdef RENDER_OPENGL
	DisplayWindowData(static_cast<RenderSystemOpenGL*>(rs)->getWindow(), cm);
	DisplaySceneGraph(cm);
	DisplayResourceList(cm, rs);
	DisplayEntityComponents(cm, rs);//Scene management
	DisplayScenes(sm);

	if (openDisplayDialogNewScene) { DisplayNewScene(cm, rs); }
	else if (openDisplayDialogSaveScene) { DisplaySaveNewScene(cm, rs, sm); }
	else if (openDisplayDialogLoadScene) { DisplayLoadScene(cm, rs, sm); }
	DisplayMenuBar(static_cast<RenderSystemOpenGL*>(rs)->getWindow(), sm);
#endif

#ifdef RENDER_DIRECTX11

	DisplayWindowData(static_cast<RenderSystemDirectX11*>(rs)->getWindow(), cm);
	DisplaySceneGraph(cm);
	DisplayResourceList(cm, rs);
	DisplayEntityComponents(cm, rs);

	DisplayMenuBar(static_cast<RenderSystemDirectX11*>(rs)->getWindow(), sm);
#endif
}

