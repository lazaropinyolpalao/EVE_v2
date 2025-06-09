#include "engine.hpp"

int main(int, char**) {
	//*/
	unsigned int const window_w = 1280, window_h = 720;
	/*/
	unsigned int const window_w = 1920, window_h = 1080;
	/**/
	Engine engine = Engine(window_w, window_h);
	Window* window = engine.getWindow();

	window->set_title("Test Shadows");

	// #### CAMERA ####
	engine.getComponentManager()->NewPerspectiveCamera(45.0f, (float)window_w / (float)window_h, 1.0f, 2000.0f);
	engine.getComponentManager()->get_principal_camera()->set_position(0.0f, 0.0f, 0.0f);
	engine.getComponentManager()->get_principal_camera()->set_pitch(0.0f);
	engine.getComponentManager()->get_principal_camera()->set_yaw(0.0f);

	engine.getRenderSystem()->resource_list_.addTextToRender("abcdefghijklmnopqrstuvwxyz", 15.0f, 15.0f, glm::vec3(1.0f, 0.0f, 0.5f), 1.0f);
	engine.getRenderSystem()->resource_list_.addTextToRender("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 15.0f, 50.0f, glm::vec3(0.5, 0.8f, 0.2f), 1.0f);

	// #### LIGHTS ####

	DirectionalLight* dir = engine.getRenderSystem()->lights_.addDirectionalLight();
	dir->position_= {-15.0f, 30.0f, -15.0f};
	dir->ambient_ = { 0.25f, 0.25f, 0.25f };
	dir->pitch_ = -120.0f;
	dir->yaw_ = 0.0f;
	dir->zfar_ = 200.0f;
	dir->znear_ = 0.1f;
	dir->UpdateProjection();
	dir->UpdateView();
	
	SpotLight* spot = engine.getRenderSystem()->lights_.addSpotLight();
	spot->position_ = { 0.0f, 25.0f, 0.0f };
	spot->pitch_ = -90.0f;
	spot->yaw_ = 0.0f;
	spot->cut_off_= 0.75f;
	spot->outer_cut_off_ = 0.5f;
	spot->constant_ = 1.0f;
	spot->linear_ = 0.0025f;
	spot->quadratic_ = 0.0001f;
	spot->fov_ = 90.0f;
	spot->znear_ = 0.2f;
	spot->zfar_ = 200.0f;
	spot->UpdateProjection();
	spot->UpdateView();

	PointLight* point = engine.getRenderSystem()->lights_.addPointLight();
	point->position_ = { 0.0f, 15.0f, 0.0f };
	point->constant_ = 1.0f;
	point->linear_ = 0.075f;
	point->quadratic_ = 0.001f;
	point->znear_ = 0.1f;
	point->zfar_ = 100.0f;
	point->UpdateProjection();
	point->UpdateView();

	//engine.getComponentManager()->lights_->spot_.back();

	//engine.getComponentManager()->lights_.addPointLight();
	//engine.getComponentManager()->lights_->directional_.back()->visible_ = false;
	//componentmanager->lights_->point_.back()->visible_ = false;

	//Load textures
	engine.AddTexture("../data/textures/wall2.jpg");
	engine.AddTexture("../data/textures/wall.jpg");
	
	//Load meshes
	std::shared_ptr<TinyObj> shared_mesh = engine.AddMesh("../data/meshes/cube.obj");
	std::shared_ptr<TinyObj> shared_capsule_mesh = engine.AddMesh("../data/meshes/capsule.obj");

	//Ground entity
	size_t ground_entity = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(50.0f, 2.0f, 50.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	//engine.getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(1.0f, 1.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	engine.getComponentManager()->get_component<RendererComponent>(ground_entity)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[0]);

	//Walls
	//*/
	size_t wall1 = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->addComponent<TransformComponent>(wall1)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, -51.0f));
	engine.getComponentManager()->get_component<RendererComponent>(wall1)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[1]);

	size_t wall2 = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->addComponent<TransformComponent>(wall2)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, 51.0f));
	engine.getComponentManager()->get_component<RendererComponent>(wall2)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[1]);

	size_t wall3 = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->addComponent<TransformComponent>(wall3)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(-51.0f, 6.0f, 0.0f));
	engine.getComponentManager()->get_component<RendererComponent>(wall3)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[1]);

	size_t wall4 = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->addComponent<TransformComponent>(wall4)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(51.0f, 6.0f, 0.0f));
	engine.getComponentManager()->get_component<RendererComponent>(wall4)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[1]);

	size_t cube = engine.getComponentManager()->NewRenderer(shared_mesh);
	engine.getComponentManager()->get_component<TransformComponent>(cube)->SetTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
	engine.getComponentManager()->get_component<RendererComponent>(cube)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[0]);

	size_t capsule = engine.getComponentManager()->NewRenderer(shared_capsule_mesh);
	engine.getComponentManager()->get_component<TransformComponent>(capsule)->SetTranslation(glm::vec3(0.0f, 20.0f, 0.0f));
	engine.getComponentManager()->get_component<RendererComponent>(capsule)->AddTexture(engine.getRenderSystem()->resource_list_.textures_[1]);

	/**/

	//Launche the gameloop
	Input input;
	float dt;
	if (window) {
	  while (window->is_open() && !window->return_last_input().escape) {
			engine.Update();

			//Smooth update, limited to framelimit
			if (window->tick()) {

				input = window->return_last_input();
				dt = (float)window->get_delta_time();
				engine.getComponentManager()->get_principal_camera()->UpdateCamera(input, dt);
				//engine.getComponentManager()->get_principal_camera()->UpdateCameraWithPad(input, dt, 1.0f);
				//Rotate the capsule
				//engine.getComponentManager()->get_component<TransformComponent>(capsule)->AddRotationY(dt*0.25f);
			}

			engine.Render();
			//printf("Input a is pressed: %s\n", (input.a ? "true" : "false"));
	  }
	}
	else {std::cout << "Something went wrong...";}

	return 0;
}