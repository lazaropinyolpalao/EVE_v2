#ifndef __render_system_directx11_H__
#define __render_system_directx11_H__ 1

#include <wrl.h>
#include <render_system.hpp>
#include <light.hpp>


class RenderSystemDirectX11 : public RenderSystem {
public:
	RenderSystemDirectX11(int window_w, int window_h);
#ifdef RENDER_DIRECTX11
	RenderSystemDirectX11(ID3D11Device* device, ID3D11DeviceContext* devCont,
		IDXGISwapChain* swap, ID3D11DepthStencilView* depthStencil, HWND hwnd, int window_w, int window_h);
#endif
	~RenderSystemDirectX11();

	virtual void Render(ComponentManager* comp);
	virtual void Update();
	Window* getWindow();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getDeviceContext();

	bool is_uwp_ = false;
private:
	RenderSystemDirectX11() = delete;
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

	Microsoft::WRL::ComPtr<ID3D11BlendState> g_BlendStateNoBlend = nullptr;
	Microsoft::WRL::ComPtr<ID3D11BlendState> g_BlendStateBlend = nullptr;

	//--Shaders
	//Default render
	Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> g_pTransProjViewConstantBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader = nullptr;

	//Constant buffer for each object
	Microsoft::WRL::ComPtr<ID3D11Buffer> objectLightInteractionConstantBuffer = nullptr;

	//Directional light
	Microsoft::WRL::ComPtr<ID3D11VertexShader> directionalLightVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> directionalLightPixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> directionalLightUniformsConstantBuffer = nullptr;

	//Spotlight
	Microsoft::WRL::ComPtr<ID3D11VertexShader> spotLightVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> spotLightPixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> spotLightUniformsConstantBuffer = nullptr;

	//Pointlight
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pointLightVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pointLightPixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pointLightUniformsConstantBuffer = nullptr;

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

	struct SpotLightUniforms {
		glm::vec3 position;
		int pad1;
		glm::vec3 direction;
		int pad2;
		glm::vec3 diffuse;
		int pad3;
		glm::vec3 specular;
		float cutOff;
		float outerCutOff;
		float constant;
		float linear_light;
		float quadratic;
	};

	struct PointLightUniforms {
		glm::vec3 position;
		int pad1;
		glm::vec3 diffuse;
		int pad2;
		glm::vec3 specular;
		float constant;
		float linear_light;
		float quadratic;
		float range;
		int pad3;
	};

	bool InitDirectXResources();
	bool InitShaders();

	//UWP
	bool InitDirectXResourcesWithWindow();
	bool InitShadersUWP();
	
};

#endif //__render_system_directx11_H__