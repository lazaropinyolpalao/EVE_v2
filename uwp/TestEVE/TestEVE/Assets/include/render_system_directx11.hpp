#ifndef __render_system_directx11_H__
#define __render_system_directx11_H__ 1

#include <wrl.h>
#include <render_system.hpp>
#include <light.hpp>


class RenderSystemXbox : public RenderSystem {
public:
	RenderSystemXbox(int window_w, int window_h);
#ifdef RENDER_DIRECTX11
	RenderSystemXbox(ID3D11Device* device, ID3D11DeviceContext* devCont,
		IDXGISwapChain* swap, ID3D11DepthStencilView* depthStencil, HWND hwnd, int window_w, int window_h);
#endif
	~RenderSystemXbox();

	virtual void Render(ComponentManager* comp);
	virtual void Update();
	Window* getWindow();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getDeviceContext();

	bool is_uwp_ = false;
private:
	RenderSystemXbox() = delete;
	/** Window of the engine */
	std::unique_ptr<Window> window_;

	Microsoft::WRL::ComPtr<ID3D11Device> g_pd3dDevice = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> g_pDepthStencil = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pDSState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_pDSView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> g_BackBuffer = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> g_pSwapChain = nullptr;
	//UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_mainRenderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> g_pInputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> g_pRasterizerState = nullptr;

	Microsoft::WRL::ComPtr<ID3D11BlendState> g_BlendState = nullptr;

	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> g_pConstantBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader = nullptr;

	Microsoft::WRL::ComPtr<ID3D11Buffer> objectLightInteractionConstantBuffer = nullptr;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> directionalLightVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> directionalLightConstantBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> directionalLightPixelShader = nullptr;

	struct PerFrame {
		glm::mat4 transform;
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct ObjectLightInteraction {
		glm::vec3 viewPos;
		bool receiveShadows;
		glm::vec3 padding;
		bool needsLight;
	};

	struct DirectionalLightUniforms {
		glm::mat4 directionalLightMatrix;
		glm::vec3 directionalPosition;
		float p1;
		glm::vec3 directionalDirection;
		float p2;
		glm::vec3 directionalAmbient;
		float p3;
		glm::vec3 directionalDiffuse;
		float p4;
		glm::vec3 directionalSpecular;
		float p5;
	};

	bool InitDirectXResources();
	bool InitShaders();



	bool InitDirectXResourcesWithWindow();
	bool InitShadersUWP();

	////## FORWARD RENDERING
	///**
	//* @brief Render the scene cubemap
	//*/
	//void render_scene_cubemap(ComponentManager* comp);
	///**
	// * @brief Render only the elements with their textures associated
	// * 
	// * @param comp ComponentManager to get the data from
	// * @param prog Program to use in the rendering of the scene
	// */
	//void render_elements_with_texture(ComponentManager* comp, Program* prog);
	///**
	// * @brief Render only the elements for the shadow depthmap
	// *
	// * @param comp ComponentManager to get the data from
	// * @param prog Program to use in the rendering of the scene
	// */
	//void render_elements_depthmap(ComponentManager* comp, Program* prog);
	///**
	// * @brief Render the elements with a directionallight
	// *
	// * @param comp ComponentManager to get the data from
	// * @param prog Program to use in the rendering of the scene
	// * @param directional The DirectionalLight to draw with
	// */
	//void render_light_elements(ComponentManager* comp, Program* prog, DirectionalLight* directional);

	///**
	//* @brief Render the elements with a spotlight
	//*
	//* @param comp ComponentManager to get the data from
	//* @param prog Program to use in the rendering of the scene
	//* @param spotlight The SpotLight to draw with
	//*/
	//void render_light_elements(ComponentManager* comp, Program* prog, SpotLight* spotlight);

	///**
	//* @brief Render the elements with a pointlight
	//*
	//* @param comp ComponentManager to get the data from
	//* @param prog Program to use in the rendering of the scene
	//* @param pointlight The PointLight to draw with
	//*/
	//void render_light_elements(ComponentManager* comp, Program* prog, PointLight* pointlight);
	///**
	// * @brief Forward rendering method
	// * 
	// * @param comp ComponentManager to get the data from
	// */
	//void ForwardRendering(ComponentManager* comp);

	///** Depthmap used in both directional and spotlight shadows */
	//std::unique_ptr<DepthMap> depthmap_directional_and_spotlight_shadows_;

	///** Depthmap used to render pointlight shadows */
	//std::unique_ptr<DepthMap> depthmap_pointlight_shadows_;

	////## DEFERRED RENDERING

	///**
	//* @brief Sets the DirectionalLight data to the corresponding shader and renders it
	//*
	//* @param prog Program to use in the rendering of the scene
	//* @param directional The DirectionalLight to draw with
	//*/
	//void render_deferred_light(Program* prog, DirectionalLight* directional);
	///**
	// * @brief Sets the SpotLight data to the corresponding shader and renders it
	// *
	// * @param prog Program to use in the rendering of the scene
	// * @param spotlight The SpotLight to draw with
	// */
	//void render_deferred_light(Program* prog, SpotLight* spotlight);

	///**
	// * @brief Sets the PointLight data to the corresponding shader and renders it
	// *
	// * @param prog Program to use in the rendering of the scene
	// * @param pointlight The PointLight to draw with
	// */
	//void render_deferred_light(Program* prog, PointLight* pointlight);

	///**
	//* @brief Render only the elements with their textures associated
	//*
	//* @param comp ComponentManager to get the data from
	//* @param prog Program to use in the rendering of the scene
	//*/
	//void render_deferred_elements_without_light(ComponentManager* comp, Program* prog);

	///**
	//* @brief Deferred rendering method.
	//*/
	//void DeferredRendering(ComponentManager* comp);

	//std::unique_ptr<DeferredFramebuffer> deferred_framebuffer_;
};

#endif //__render_system_directx11_H__