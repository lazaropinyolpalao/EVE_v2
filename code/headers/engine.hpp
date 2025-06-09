#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__  1

#include <filesystem>
#include <time.h>

#ifdef RENDER_DIRECTX11
  #include <wrl.h>
#endif

#include <defines.hpp>
#include <component_system.hpp>
#include <scene_manager.hpp>
#include <imgui_functions.hpp>
#include <boss.hpp>

#ifdef RENDER_OPENGL
    #include <framebuffer_to_texture.hpp>
    #include <render_system_opengl.hpp>
#endif
#ifdef RENDER_DIRECTX11
    #include <d3d11.h>
    #include <d3dcompiler.h>
#endif

/**
 * @brief Base of the game engine, <B>ONE NEEDS TO BE CREATED BEFORE ANYTHING ELSE ON EXECUTION</B>
 */
class Engine {

public:

    /**
     * @brief Launch the needed dependencies of the engine. Called <B>automatically at the constructor of the engine</B>
     * 
     * @return int 0 Started correctly
     */
    int startUp();

    /**
     * @brief Close the dependencies of the engine. Called <B>automatically at the destructor of the engine</B>
     * 
     * @return int 0 Closed correctly
     */
    int shutdown();

    /**
     * @brief Gameloop that will be executed every frame.
     */
    void GameLoop();

    /**
     * @brief Returns a pointer to the active window of the engine
     * 
     * @return Window* Pointer to the window object
     */
    Window* getWindow();

    /**
    * @brief Returns a pointer to the component manager of the engine
    *
    * @return ComponentManager* Pointer to the component manager object
    */
    ComponentManager* getComponentManager();

    /**
    * @brief Returns a pointer to the scene manager of the engine
    *
    * @return SceneManager* Pointer to the scene manager object
    */
    SceneManager* getSceneManager();

    /**
     * @brief Returns a pointer to the render system of the engine
     *
     * @return RenderSystem* Pointer to the render system
     */
    RenderSystem* getRenderSystem();

    /**
     * @brief Returns a pointer to the job system boss
     * 
     * @return Boss* Pointer to the boss
     */
    Boss* getBossSystem();

    /**
    * @brief Loads a new mesh based on the filepath given
    * 
    * @param filepath Route to the resource
    */
    std::shared_ptr<TinyObj> AddMesh(std::string filepath);
    /**
    * @brief Loads a new mesh based on the filepath given
    *
    * @param filepath Route to the resource
    */
    void AddTexture(std::string filepath);

    /**
     * @brief Save all contents of the scene into a already created scene, which functions as un "update" sort of function
     * 
     * @param scene_id Which scene to save (update)
     */
    //void SaveScene(unsigned int scene_id);

    /**
     * @brief Encapsulation of the functions that update the engine gameloop
     */
    void Update();

    /**
     * @brief Encapsulation of the functions that render the engine window
     */
    void Render();

    Engine() = delete;
    Engine(int window_w, int window_h);
#ifdef RENDER_DIRECTX11
    Engine(ID3D11Device* device, ID3D11DeviceContext* devCont, 
        IDXGISwapChain* swap, ID3D11DepthStencilView* depthStencil, HWND hwnd, int window_w, int window_h);
#endif
    ~Engine();

private:

    /** Window of the engine */
    std::unique_ptr<Window> window_;
    /** Component manager of the window */
    std::unique_ptr<ComponentManager> component_manager_;
    /** Scene manager to save and load a scene from the database */
    std::unique_ptr<SceneManager> scene_manager_;
    /** Boss system to multithread */
    std::unique_ptr<Boss> boss_system_;

    /** Render system that will take care of the displaying of elements */
    std::unique_ptr<RenderSystem> render_system_;

    /** OpenAL Device */
    ALCdevice* audio_device_;
    /** OpenAL Context */
    ALCcontext* audio_context_;
    
};

#endif //__ENGINE_HPP__