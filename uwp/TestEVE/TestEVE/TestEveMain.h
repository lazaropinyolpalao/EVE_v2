#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <engine.hpp>


struct InputPad {
	/** Xbox input */
  //Buttons
	bool button_x = false;
	bool button_y = false;
	bool button_a = false;
	bool button_b = false;
	bool button_rb = false;
	bool button_lb = false;

	//D-pad
	bool dpad_up = false;
	bool dpad_down = false;
	bool dpad_left = false;
	bool dpad_right = false;

	//Specials
	bool start = false;
	bool select = false;

	//Trigger pressed or not, and force
	bool trigger_r = false;
	bool trigger_l = false;
	char trigger_r_force = 0;
	char trigger_l_force = 0;

	//Sticks
	bool left_thumb = false;
	bool right_thumb = false;
	float l_stick_x = 0.0f;
	float l_stick_y = 0.0f;
	float l_stick_norm_x = 0.0f;
	float l_stick_norm_y = 0.0f;

	float r_stick_x = 0.0f;
	float r_stick_y = 0.0f;
	float r_stick_norm_x = 0.0f;
	float r_stick_norm_y = 0.0f;
};

// Presenta contenido Direct2D y 3D en la pantalla.
namespace TestEve
{
	class TestEveMain : public DX::IDeviceNotify
	{
	public:
		TestEveMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		TestEveMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, HWND hwnd);
		~TestEveMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();
	private:
		std::unique_ptr<Engine> engine;
		// Puntero almacenado en caché para los recursos del dispositivo.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Sustituir con sus propios representadores de contenido.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// Temporizador de bucle de representación.
		DX::StepTimer m_timer;
	};
}