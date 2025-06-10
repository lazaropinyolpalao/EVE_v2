#include "engine.hpp"


int main(int, char**) {

	unsigned int const window_w = 1280, window_h = 720;

	Engine engine = Engine(window_w, window_h);	
	Boss* b = engine.getBossSystem();
	Window* window = engine.getWindow();
	window->set_title("EVE Demo");

	ComponentManager* componentmanager = engine.getComponentManager();

	//componentmanager->resource_list_.toggleCubeMap();

	// #### CAMERA ####
	size_t cam_id = engine.getComponentManager()->NewPerspectiveCamera(45.0f, (float)window_w / (float)window_h, 1.0f, 5000.0f);
	CameraComponent* cam = engine.getComponentManager()->get_component<CameraComponent>(cam_id);
	cam->set_position(0.0f, 400.0f, 1250.0f);
	cam->set_pitch(-10.0f);
	cam->set_yaw(-90.0f);
	cam->set_speed(10.0f);

	// #### LIGHT ####

	DirectionalLight* directional = engine.getRenderSystem()->lights_.addDirectionalLight();
	directional->set_pitch(-180.0f);
	directional->set_yaw(90.0f);
	directional->set_ambient(0.15f, 0.15f, 0.15f);

	SpotLight* spot1 = engine.getRenderSystem()->lights_.addSpotLight();
	spot1->set_pitch(-70.0f);
	spot1->set_yaw(-60.0f);
	spot1->set_position(-300.0f, 60.0f, -85.0f);
	spot1->set_diffuse(0.0f, 1.0f, 1.0f);
	spot1->set_cut_off(0.65f);
	spot1->set_outer_cut_off(0.5f);
	spot1->set_near(2.0f);
	spot1->set_far(200.0f);

	SpotLight* spot2 = engine.getRenderSystem()->lights_.addSpotLight();
	spot2->set_pitch(-110.0f);
	spot2->set_yaw(60.0f);
	spot2->set_position(300.0f, 60.0f, -85.0f);
	spot2->set_diffuse(1.0f, 0.0f, 1.0f);
	spot2->set_cut_off(0.65f);
	spot2->set_outer_cut_off(0.5f);
	spot2->set_near(2.0f);
	spot2->set_far(200.0f);

	SpotLight* spot3 = engine.getRenderSystem()->lights_.addSpotLight();
	spot3->set_pitch(-90.0f);
	spot3->set_yaw(0.0f);
	spot3->set_position(0.0f, 150.0f, -250.0f);
	spot3->set_diffuse(1.0f, 1.0f, 0.0f);
	spot3->set_cut_off(0.65f);
	spot3->set_outer_cut_off(0.5f);
	spot3->set_near(20.0f);
	spot3->set_far(200.0f);

	SpotLight* spot4 = engine.getRenderSystem()->lights_.addSpotLight();
	spot4->set_pitch(-120.0f);
	spot4->set_yaw(90.0f);
	spot4->set_position(0.0f, 60.0f, -10.0f);
	spot4->set_diffuse(0.0f, 1.0f, 0.0f);
	spot4->set_cut_off(0.6f);
	spot4->set_outer_cut_off(0.5f);
	spot4->set_near(2.0f);
	spot4->set_far(200.0f);

	SpotLight* spot5 = engine.getRenderSystem()->lights_.addSpotLight();
	spot5->set_pitch(-145.0f);
	spot5->set_yaw(-45.0f);
	spot5->set_position(-475.0f, 150.0f, 150.0f);
	spot5->set_cut_off(0.75f);
	spot5->set_outer_cut_off(0.65f);
	spot5->set_linear(0.0002f);
	spot5->set_quadratic(0.0f);
	spot5->set_near(2.0f);
	spot5->set_far(200.0f);

	SpotLight* spot6 = engine.getRenderSystem()->lights_.addSpotLight();
	spot6->set_pitch(-120.0f);
	spot6->set_yaw(-90.0f);
	spot6->set_position(-250.0f, 150.0f, 175.0f);
	spot6->set_cut_off(0.75f);
	spot6->set_outer_cut_off(0.65f);
	spot6->set_linear(0.0002f);
	spot6->set_quadratic(0.0f);
	spot6->set_near(2.0f);
	spot6->set_far(200.0f);

	SpotLight* spot7 = engine.getRenderSystem()->lights_.addSpotLight();
	spot7->set_pitch(-120.0f);
	spot7->set_yaw(-90.0f);
	spot7->set_position(250.0f, 150.0f, 175.0f);
	spot7->set_cut_off(0.75f);
	spot7->set_outer_cut_off(0.65f);
	spot7->set_linear(0.0002f);
	spot7->set_quadratic(0.0f);
	spot7->set_near(2.0f);
	spot7->set_far(200.0f);

	SpotLight* spot8 = engine.getRenderSystem()->lights_.addSpotLight();
	spot8->set_pitch(-145.0f);
	spot8->set_yaw(-135.0f);
	spot8->set_position(475.0f, 150.0f, 150.0f);
	spot8->set_cut_off(0.75f);
	spot8->set_outer_cut_off(0.65f);
	spot8->set_linear(0.0002f);
	spot8->set_quadratic(0.0f);
	spot8->set_near(2.0f);
	spot8->set_far(200.0f);


	// #### MESHES ####

	std::mutex mesh_access_mutex;
	std::mutex texture_access_mutex;

	//Meshes jobs
	std::function<void()> task_stage = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/stage.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_drumset = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/drumset.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_fence = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/fence.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_sphere = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/sphere.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_capsule = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/capsule.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_cube = [&mesh_access_mutex, &engine]() {
		std::shared_ptr<TinyObj> v = std::make_shared<TinyObj>();
		v->LoadObj("../data/meshes/cube.obj");
		std::unique_lock<std::mutex> lk(mesh_access_mutex);
		engine.getRenderSystem()->resource_list_.meshes_.push_back(std::move(v));
		lk.unlock();
	};

	//Textures jobs
	std::function<void()> task_texture_green = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/green.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_blue = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/blue.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_red = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/red.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_yellow = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/yellow.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_orange = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/orange.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
		};

	std::function<void()> task_texture_brown = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/brown.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_stage = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/stage.png");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_drumset = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/drumset.png");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_wall = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/wall.jpg");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::function<void()> task_texture_fence = [&texture_access_mutex, &engine]() {
		std::shared_ptr<Texture> v = std::make_shared<Texture>();
		v->LoadTextureNoInit("../data/textures/fence.png");
		std::unique_lock<std::mutex> lk(texture_access_mutex);
		engine.getRenderSystem()->resource_list_.textures_.push_back(std::move(v));
		lk.unlock();
	};

	std::vector<std::future<void>> futures;

	// #### MESHES ####
	futures.emplace_back(b->add(std::move(task_drumset)));
	futures.emplace_back(b->add(std::move(task_stage)));
	futures.emplace_back(b->add(std::move(task_fence)));
	futures.emplace_back(b->add(std::move(task_sphere)));
	futures.emplace_back(b->add(std::move(task_capsule)));
	futures.emplace_back(b->add(std::move(task_cube)));

	// #### TEXTURES ####
	futures.emplace_back(b->add(std::move(task_texture_green)));
	futures.emplace_back(b->add(std::move(task_texture_blue)));
	futures.emplace_back(b->add(std::move(task_texture_red)));
	futures.emplace_back(b->add(std::move(task_texture_yellow)));
	futures.emplace_back(b->add(std::move(task_texture_orange)));
	futures.emplace_back(b->add(std::move(task_texture_brown)));
	futures.emplace_back(b->add(std::move(task_texture_stage)));
	futures.emplace_back(b->add(std::move(task_texture_drumset)));
	futures.emplace_back(b->add(std::move(task_texture_wall)));
	futures.emplace_back(b->add(std::move(task_texture_fence)));

	//Lock process until all the jobs are done
	printf("\n\nWaiting for jobs to complete...\n\n");
	for (unsigned int i = 0; i < futures.size(); ++i) {
		futures[i].wait();
	}

	for (unsigned int i = 0; i < engine.getRenderSystem()->resource_list_.meshes_.size(); i++) {
		engine.getRenderSystem()->resource_list_.meshes_.at(i)->InitBuffer();
	}

	for (unsigned int i = 0; i < engine.getRenderSystem()->resource_list_.textures_.size(); i++) {
		engine.getRenderSystem()->resource_list_.textures_.at(i)->InitTexture();
	}

	//Meshes
	std::shared_ptr<TinyObj> cube_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("cube");
	std::shared_ptr<TinyObj> stage_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("stage");
	std::shared_ptr<TinyObj> drumset_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("drumset");
	std::shared_ptr<TinyObj> capsule_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("capsule");
	std::shared_ptr<TinyObj> sphere_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("sphere");
	
	//Textures
	std::shared_ptr<Texture> texture_stage = engine.getRenderSystem()->resource_list_.getTextureByName("stage");
	std::shared_ptr<Texture> texture_drumset = engine.getRenderSystem()->resource_list_.getTextureByName("drumset");
	std::shared_ptr<Texture> texture_wall = engine.getRenderSystem()->resource_list_.getTextureByName("wall");
	std::shared_ptr<Texture> texture_fence = engine.getRenderSystem()->resource_list_.getTextureByName("fence");
	std::shared_ptr<Texture> texture_brown = engine.getRenderSystem()->resource_list_.getTextureByName("brown");

	std::shared_ptr<Texture> texture_green = engine.getRenderSystem()->resource_list_.getTextureByName("green");
	std::shared_ptr<Texture> texture_blue = engine.getRenderSystem()->resource_list_.getTextureByName("blue");
	std::shared_ptr<Texture> texture_red = engine.getRenderSystem()->resource_list_.getTextureByName("red");
	std::shared_ptr<Texture> texture_yellow = engine.getRenderSystem()->resource_list_.getTextureByName("yellow");
	std::shared_ptr<Texture> texture_orange = engine.getRenderSystem()->resource_list_.getTextureByName("orange");



	// Stage
	size_t stage_entity = engine.getComponentManager()->NewRenderer(stage_shared_mesh);
	TransformComponent* stage_transform = engine.getComponentManager()->get_component<TransformComponent>(stage_entity)->SetScale(0.5f, 0.5f, 0.5f);
	RendererComponent* stage_renderer = engine.getComponentManager()->get_component<RendererComponent>(stage_entity)->AddTexture(texture_stage);

	// Drumkit
	size_t drumkit_entity = engine.getComponentManager()->NewRenderer(drumset_shared_mesh);
	TransformComponent* drumkit_transform = engine.getComponentManager()->get_component<TransformComponent>(drumkit_entity)->SetScale(0.75f, 0.75f, 0.75f)->SetTranslation(0.0f, 17.0f, -250.0f);
	RendererComponent* drumkit_renderer = engine.getComponentManager()->get_component<RendererComponent>(drumkit_entity)->AddTexture(texture_drumset);

	// Ground
	size_t ground_entity = engine.getComponentManager()->NewRenderer(cube_shared_mesh);
	TransformComponent* ground_transform = engine.getComponentManager()->get_component<TransformComponent>(ground_entity)->SetScale(1300.0f, 1.0f, 1500.0f)->SetTranslation(0.0f, -80.0f, 500.0f);
	RendererComponent* ground_renderer = engine.getComponentManager()->get_component<RendererComponent>(ground_entity)->AddTexture(texture_wall);

	// Fences
	float pos_x = -500.0f;
	std::shared_ptr<TinyObj> fence_shared_mesh = engine.getRenderSystem()->resource_list_.getMeshByName("fence");
	for (int i = 0; i < 11; i++) {
		size_t fence_entity = engine.getComponentManager()->NewRenderer(fence_shared_mesh);
		TransformComponent* fence_transform = engine.getComponentManager()->get_component<TransformComponent>(fence_entity)->SetScale(5.0f, 5.0f, 5.0f)->AddRotationY(90.0f)->SetTranslation(pos_x, -53.0f, 150.0f);
		RendererComponent* fence_renderer = engine.getComponentManager()->get_component<RendererComponent>(fence_entity)->AddTexture(texture_fence);
		pos_x += 100.0f;
	}

	// Musics
	std::vector<size_t> musics;

	std::shared_ptr<Texture> colors[5] = { texture_green, texture_blue, texture_red, texture_yellow, texture_orange };

	for (int i = 0; i < 4; i++) {
		size_t body_entity = engine.getComponentManager()->NewRenderer(capsule_shared_mesh);
		size_t head_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		size_t right_hand_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		size_t left_hand_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		musics.push_back(body_entity);

		TransformComponent* body_transform = engine.getComponentManager()->get_component<TransformComponent>(body_entity)->SetScale(10.0f, 8.0f, 10.0f)->AddRotation(90.0f, 180.0f, 0.0f);
		switch (i) {
			case 0: body_transform->SetTranslation(-275.0f, 15.0f, -120.0f); break;
			case 1: body_transform->SetTranslation(0.0f, 15.0f, -45.0f); break;
			case 2: body_transform->SetTranslation(275.0f, 15.0f, -120.f); break;
			case 3: body_transform->SetTranslation(-11.0f, 85.0f, -290.0f); break;
		}

		TransformComponent* head_transform = engine.getComponentManager()->get_component<TransformComponent>(head_entity)->SetScale(0.75f, 0.75f, 0.75f)->SetTranslation(0.0f, 0.0f, -2.5f);
		TransformComponent* right_hand_transform = engine.getComponentManager()->get_component<TransformComponent>(right_hand_entity)->SetScale(0.33f, 0.33f, 0.33f)->SetTranslation(1.0f, -1.0f, 0.0f);
		TransformComponent* left_hand_transform = engine.getComponentManager()->get_component<TransformComponent>(left_hand_entity)->SetScale(0.33f, 0.33f, 0.33f)->SetTranslation(-1.0f, -1.0f, 0.0);

		int texture_index = rand() % 5;
		RendererComponent* body_renderer = engine.getComponentManager()->get_component<RendererComponent>(body_entity)->AddTexture(colors[texture_index]);
		RendererComponent* head_renderer = engine.getComponentManager()->get_component<RendererComponent>(head_entity)->AddTexture(texture_brown);
		RendererComponent* right_hand_renderer = engine.getComponentManager()->get_component<RendererComponent>(right_hand_entity)->AddTexture(texture_brown);
		RendererComponent* left_hand_renderer = engine.getComponentManager()->get_component<RendererComponent>(left_hand_entity)->AddTexture(texture_brown);

		engine.getComponentManager()->make_parent(body_entity, head_entity);
		engine.getComponentManager()->make_parent(body_entity, left_hand_entity);
		engine.getComponentManager()->make_parent(body_entity, right_hand_entity);
	}

	// Spectators
	std::vector<size_t> spectators;

	const int kNumSpectators = 30;
	for (int i = 0; i < kNumSpectators; i++) {

		size_t body_entity = engine.getComponentManager()->NewRenderer(capsule_shared_mesh);
		size_t head_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		size_t right_hand_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		size_t left_hand_entity = engine.getComponentManager()->NewRenderer(sphere_shared_mesh);
		spectators.push_back(body_entity);

		pos_x = (float) (-750 + (rand() % (750 - -750)));
		float pos_z = (float) (250 + (rand() % (1250 - 250)));
		TransformComponent* body_transform = engine.getComponentManager()->get_component<TransformComponent>(body_entity)->SetScale(10.0f, 8.0f, 10.0f)->AddRotationX(90.0f)->SetTranslation(pos_x, -55.0f, pos_z);
		TransformComponent* head_transform = engine.getComponentManager()->get_component<TransformComponent>(head_entity)->SetScale(0.75f, 0.75f, 0.75f)->SetTranslation(0.0f, 0.0f, -2.5f);
		TransformComponent* right_hand_transform = engine.getComponentManager()->get_component<TransformComponent>(right_hand_entity)->SetScale(0.33f, 0.33f, 0.33f)->SetTranslation(1.0f, -1.0f, 0.0f);
		TransformComponent* left_hand_transform = engine.getComponentManager()->get_component<TransformComponent>(left_hand_entity)->SetScale(0.33f, 0.33f, 0.33f)->SetTranslation(-1.0f, -1.0f, 0.0);

		int texture_index = rand() % 5;
		RendererComponent* body_renderer = engine.getComponentManager()->get_component<RendererComponent>(body_entity)->AddTexture(colors[texture_index]);
		RendererComponent* head_renderer = engine.getComponentManager()->get_component<RendererComponent>(head_entity)->AddTexture(texture_brown);
		RendererComponent* right_hand_renderer = engine.getComponentManager()->get_component<RendererComponent>(right_hand_entity)->AddTexture(texture_brown);
		RendererComponent* left_hand_renderer = engine.getComponentManager()->get_component<RendererComponent>(left_hand_entity)->AddTexture(texture_brown);

		engine.getComponentManager()->make_parent(body_entity, head_entity);
		engine.getComponentManager()->make_parent(body_entity, left_hand_entity);
		engine.getComponentManager()->make_parent(body_entity, right_hand_entity);

	}

	// #### AUIDO ####

	engine.getRenderSystem()->resource_list_.addAudioFile("../data/audio/all_star.wav")->play();
	engine.getRenderSystem()->resource_list_.addAudioFile("../data/audio/welcome_to_the_jungle.wav");
	engine.getRenderSystem()->resource_list_.addAudioFile("../data/audio/song_2.wav");
	engine.getRenderSystem()->resource_list_.addAudioFile("../data/audio/seven_nation_army.wav");
	engine.getRenderSystem()->resource_list_.addAudioFile("../data/audio/rock_you_like_a_hurricane.wav");

	// #### TEXT RENDERER ####

	engine.getRenderSystem()->resource_list_.addTextToRender("WASD to move camera", 10.0f, 30.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.5f);
	engine.getRenderSystem()->resource_list_.addTextToRender("Mouse right button to rotate camera", 10.0f, 10.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.5f);

	float delta_time = 0.0;
	Input input;
	float dt;

	std::vector<TransformComponent*> musics_transforms = engine.getComponentManager()->GetComponentsOfEntities<TransformComponent>(musics);
	std::vector<TransformComponent*> spectators_transforms = engine.getComponentManager()->GetComponentsOfEntities<TransformComponent>(spectators);

	struct TempStruct {
		glm::vec3 original_pos;
		int temp_speed;
		TransformComponent* t;
	};

	std::map<size_t, TempStruct> positions_music;
	std::map<size_t, TempStruct> positions_spec;

	for (unsigned int i = 0; i < musics.size(); ++i) {
		positions_music[musics.at(i)].original_pos = musics_transforms.at(i)->GetPosition();
		positions_music[musics.at(i)].t = musics_transforms.at(i);
		positions_music[musics.at(i)].temp_speed = rand() % 10;
	}

	for (unsigned int i = 0; i < spectators.size(); ++i) {
		positions_spec[spectators.at(i)].original_pos = spectators_transforms.at(i)->GetPosition();
		positions_spec[spectators.at(i)].t = spectators_transforms.at(i);
		positions_spec[spectators.at(i)].temp_speed = (rand() % 10) + 1;
	}

	//Launche the gameloop
	float frames = 0;
	if (window) {
		while (window->is_open()) {

			engine.Update();

			if (window->tick()) {

				input = window->return_last_input();
				dt = (float)window->get_delta_time(); 

				componentmanager->get_principal_camera()->UpdateCamera(input, dt);

				float s = sinf(frames);
				float c = cosf(frames);

				float sin = (s * 3.0f);
				float cos = (c * 3.0f);

				for (int i = 0; i < musics.size(); i++) {
					glm::vec3 pos = positions_music[musics.at(i)].original_pos;
					pos.y += sin / ((float)positions_music[musics.at(i)].temp_speed + 0.1f);
					positions_music[musics.at(i)].t->SetTranslation(pos);
				}

				for (int i = 0; i < spectators.size(); i++) {
					glm::vec3 pos = positions_spec[spectators.at(i)].original_pos;
					pos.y += cos / ((float)positions_spec[spectators.at(i)].temp_speed + 0.1f);
					positions_spec[spectators.at(i)].t->SetTranslation(pos);
				}

				spot1->set_yaw(spot1->get_yaw() + c);
				spot2->set_yaw(spot2->get_yaw() - c);

				spot4->set_pitch(spot4->get_pitch() + s);

				frames+=0.25f;
			}			

			engine.Render();
		}
	}
	else { std::cout << "Something went wrong..."; }

	for (unsigned int i = 0; i < engine.getRenderSystem()->resource_list_.audios_.size(); i++) {
		engine.getRenderSystem()->resource_list_.audios_.at(i)->destroy();
	}

	return 0;
}






