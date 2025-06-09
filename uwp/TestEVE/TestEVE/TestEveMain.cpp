#include "pch.h"
#include "TestEveMain.h"
#include "Common\DirectXHelper.h"
#include <winrt/Windows.Gaming.Input.h>

#include <winrt/Windows.UI.Core.h>

using namespace TestEve;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Carga e inicializa los activos de la aplicación cuando se carga la aplicación.
TestEveMain::TestEveMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{

	// TODO: Reemplácelo por la inicialización del contenido de su aplicación.
	//m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	// Registrarse para recibir notificación si el dispositivo se pierde o se vuelve a crear
	m_deviceResources->RegisterDeviceNotify(this);
	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	//Custom elements

	ID3D11Device* device = m_deviceResources->GetD3D11Device();
	ID3D11DeviceContext* device_cont;
	IDXGISwapChain* swap = m_deviceResources->GetSwapChain();

	ID3D11DepthStencilView* depth = m_deviceResources->GetDepthStencilView();
	depth->GetDevice(&device);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swap->GetDesc(&swapChainDesc);

	device->GetImmediateContext(&device_cont);

	//*/
	unsigned int const window_w = 1280, window_h = 720;
	/*/
	unsigned int const window_w = 1920, window_h = 1080;
	/**/



	Windows::UI::Core::CoreWindow::GetForCurrentThread()->Activate();
	engine = std::make_unique<Engine>(device, device_cont, swap, depth, m_deviceResources->hwnd, 1280, 720);

	//Init custom engine components
	Window* window = engine->getWindow();

	window->set_title("Test Shadows");

	// #### CAMERA ####
	engine->getComponentManager()->NewPerspectiveCamera(45.0f, (float)window_w / (float)window_h, 1.0f, 2000.0f);
	engine->getComponentManager()->get_principal_camera()->position_ = glm::vec3(0.0f, 0.0f, 0.0f);
	engine->getComponentManager()->get_principal_camera()->pitch_ = 0.0f;
	engine->getComponentManager()->get_principal_camera()->yaw_ = 0.0f;


	// #### LIGHTS ####

	DirectionalLight* dir = engine->getRenderSystem()->lights_.addDirectionalLight();
	dir->position_ = { -15.0f, 30.0f, -15.0f };
	dir->ambient_ = { 0.25f, 0.25f, 0.25f };
	dir->pitch_ = -120.0f;
	dir->yaw_ = 0.0f;
	dir->zfar_ = 200.0f;
	dir->znear_ = 0.1f;
	dir->UpdateProjection();
	dir->UpdateView();

	SpotLight* spot = engine->getRenderSystem()->lights_.addSpotLight();
	spot->position_ = { 0.0f, 25.0f, 0.0f };
	spot->pitch_ = -90.0f;
	spot->yaw_ = 0.0f;
	spot->cut_off_ = 0.75f;
	spot->outer_cut_off_ = 0.5f;
	spot->constant_ = 1.0f;
	spot->linear_ = 0.0025f;
	spot->quadratic_ = 0.0001f;
	spot->fov_ = 90.0f;
	spot->znear_ = 0.2f;
	spot->zfar_ = 200.0f;
	spot->UpdateProjection();
	spot->UpdateView();

	PointLight* point = engine->getRenderSystem()->lights_.addPointLight();
	point->position_ = { 0.0f, 15.0f, 0.0f };
	point->constant_ = 1.0f;
	point->linear_ = 0.075f;
	point->quadratic_ = 0.001f;
	point->znear_ = 0.1f;
	point->zfar_ = 100.0f;
	point->UpdateProjection();
	point->UpdateView();

	//Load textures
	engine->AddTexture("wall2.jpg");
	engine->AddTexture("wall.jpg");

	//Load meshes	
	std::shared_ptr<TinyObj> shared_mesh = engine->AddMesh("cube.obj");
	std::shared_ptr<TinyObj> shared_capsule_mesh = engine->AddMesh("capsule.obj");

	//Ground entity
	size_t ground_entity = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(50.0f, 2.0f, 50.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	//engine.getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(1.0f, 1.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(ground_entity)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[0]);

	//Walls
	//*/
	size_t wall1 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall1)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, -51.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall1)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall2 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall2)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, 51.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall2)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall3 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall3)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(-51.0f, 6.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall3)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall4 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall4)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(51.0f, 6.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall4)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t cube = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->get_component<TransformComponent>(cube)->SetTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(cube)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[0]);

	size_t capsule = engine->getComponentManager()->NewRenderer(shared_capsule_mesh);
	engine->getComponentManager()->get_component<TransformComponent>(capsule)->SetTranslation(glm::vec3(0.0f, 20.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(capsule)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

}

TestEveMain::TestEveMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, HWND hwnd) :
	m_deviceResources(deviceResources)
{

	// TODO: Reemplácelo por la inicialización del contenido de su aplicación.
	//m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	// Registrarse para recibir notificación si el dispositivo se pierde o se vuelve a crear
	m_deviceResources->RegisterDeviceNotify(this);
	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));
	
	//Custom elements

	ID3D11Device* device = m_deviceResources->GetD3D11Device();
	ID3D11DeviceContext* device_cont;
	IDXGISwapChain *swap = m_deviceResources->GetSwapChain();

	ID3D11DepthStencilView * depth = m_deviceResources->GetDepthStencilView();
	depth->GetDevice(&device);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swap->GetDesc(&swapChainDesc);

	device->GetImmediateContext(&device_cont);

	//*/
	unsigned int const window_w = 1280, window_h = 720;
	/*/
	unsigned int const window_w = 1920, window_h = 1080;
	/**/

	

	engine = std::make_unique<Engine>(device, device_cont, swap, depth, hwnd, 1280, 720);

	//Init custom engine components
	Window* window = engine->getWindow();

	window->set_title("Test Shadows");

	// #### CAMERA ####
	engine->getComponentManager()->NewPerspectiveCamera(45.0f, (float)window_w / (float)window_h, 1.0f, 2000.0f);
	engine->getComponentManager()->get_principal_camera()->position_ = glm::vec3(0.0f, 0.0f, 0.0f);
	engine->getComponentManager()->get_principal_camera()->pitch_ = 0.0f;
	engine->getComponentManager()->get_principal_camera()->yaw_ = 0.0f;


	// #### LIGHTS ####

	DirectionalLight* dir = engine->getRenderSystem()->lights_.addDirectionalLight();
	dir->position_ = { -15.0f, 30.0f, -15.0f };
	dir->ambient_ = { 0.25f, 0.25f, 0.25f };
	dir->pitch_ = -120.0f;
	dir->yaw_ = 0.0f;
	dir->zfar_ = 200.0f;
	dir->znear_ = 0.1f;
	dir->UpdateProjection();
	dir->UpdateView();

	SpotLight* spot = engine->getRenderSystem()->lights_.addSpotLight();
	spot->position_ = { 0.0f, 25.0f, 0.0f };
	spot->pitch_ = -90.0f;
	spot->yaw_ = 0.0f;
	spot->cut_off_ = 0.75f;
	spot->outer_cut_off_ = 0.5f;
	spot->constant_ = 1.0f;
	spot->linear_ = 0.0025f;
	spot->quadratic_ = 0.0001f;
	spot->fov_ = 90.0f;
	spot->znear_ = 0.2f;
	spot->zfar_ = 200.0f;
	spot->UpdateProjection();
	spot->UpdateView();

	PointLight* point = engine->getRenderSystem()->lights_.addPointLight();
	point->position_ = { 0.0f, 15.0f, 0.0f };
	point->constant_ = 1.0f;
	point->linear_ = 0.075f;
	point->quadratic_ = 0.001f;
	point->znear_ = 0.1f;
	point->zfar_ = 100.0f;
	point->UpdateProjection();
	point->UpdateView();

	//Load textures
	engine->AddTexture("wall2.jpg");
	engine->AddTexture("wall.jpg");

	//Load meshes	
	std::shared_ptr<TinyObj> shared_mesh = engine->AddMesh("cube.obj");
	std::shared_ptr<TinyObj> shared_capsule_mesh = engine->AddMesh("capsule.obj");

	//Ground entity
	size_t ground_entity = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(50.0f, 2.0f, 50.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	//engine.getComponentManager()->addComponent<TransformComponent>(ground_entity)->SetScale(glm::vec3(1.0f, 1.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(ground_entity)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[0]);

	//Walls
	//*/
	size_t wall1 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall1)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, -51.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall1)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall2 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall2)->SetScale(glm::vec3(50.0f, 4.0f, 1.0f))->SetTranslation(glm::vec3(0.0f, 6.0f, 51.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall2)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall3 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall3)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(-51.0f, 6.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall3)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t wall4 = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->addComponent<TransformComponent>(wall4)->SetScale(glm::vec3(1.0f, 4.0f, 50.0f))->SetTranslation(glm::vec3(51.0f, 6.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(wall4)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);

	size_t cube = engine->getComponentManager()->NewRenderer(shared_mesh);
	engine->getComponentManager()->get_component<TransformComponent>(cube)->SetTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(cube)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[0]);

	size_t capsule = engine->getComponentManager()->NewRenderer(shared_capsule_mesh);
	engine->getComponentManager()->get_component<TransformComponent>(capsule)->SetTranslation(glm::vec3(0.0f, 20.0f, 0.0f));
	engine->getComponentManager()->get_component<RendererComponent>(capsule)->AddTexture(engine->getRenderSystem()->resource_list_.textures_[1]);
}

TestEveMain::~TestEveMain()
{
	// Anular el registro de notificación del dispositivo
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Actualiza el estado de la aplicación cuando cambia el tamaño de la ventana (p. ej., un cambio de orientación del dispositivo)
void TestEveMain::CreateWindowSizeDependentResources() 
{
	// TODO: Reemplácelo por la inicialización dependiente del tamaño del contenido de su aplicación.
	//m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Actualiza el estado de la aplicación una vez por marco.
void TestEveMain::Update() {
	// Actualizar los objetos de la escena.
	m_timer.Tick([&](){
		engine->Update();
		Input input = engine->getWindow()->return_last_input();
		//float dt = (float)engine->getWindow()->get_delta_time();
		float dt = (float)m_timer.GetElapsedSeconds();
		engine->getComponentManager()->get_principal_camera()->UpdateCamera(input, dt, 10.0f);

		m_fpsTextRenderer->Update(m_timer);
	});
}

// Presenta el marco actual de acuerdo con el estado actual de la aplicación.
// Devuelve true si se ha presentado el marco y está listo para ser mostrado.
bool TestEveMain::Render() 
{
	// No intente presentar nada antes de la primera actualización.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Restablecer la ventanilla para que afecte a toda la pantalla.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Restablecer los valores de destino de presentación en la pantalla.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Borrar el búfer de reserva y la vista de galería de símbolos de profundidad.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Presentar los objetos de la escena.
	// TODO: Reemplácelo por las funciones de representación de contenido de su aplicación.
	// 
	//engine->Render();
	//m_sceneRenderer->Render();

	m_fpsTextRenderer->Render();
	engine->Render();

	return true;
}

// Notifica a los representadores que deben liberarse recursos del dispositivo.
void TestEveMain::OnDeviceLost()
{
	//m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifica a los representadores que los recursos del dispositivo pueden volver a crearse.
void TestEveMain::OnDeviceRestored()
{
	//m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
