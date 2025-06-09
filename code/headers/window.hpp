#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__  1



#ifdef RENDER_OPENGL
  #include <time.h>
  #include <memory>
  #include "GL/glew.h"
  #include <GLFW/glfw3.h>
  #define GLFW_EXPOSE_NATIVE_WIN32
  #include <GLFW/glfw3native.h>
#endif
#ifdef RENDER_DIRECTX11
  #include <d3d11.h>
  #include <d3dcompiler.h>
  #include <time.h>
  #include <memory>
  #include <chrono>
#endif


#define INPUT_DEADZONE 16383 
#include <Xinput.h>

/**
 * @brief Structure that stores all the inputs that can be read from keyboard and mouse
 */
struct Input {

  /** Key UP */
  bool up = false;
  /** Key DOWN */
  bool down = false;
  /** Key LEFT */
  bool left = false;
  /** Key RIGHT */
  bool right = false;

  /** Key W */
  bool w = false;
  /** Key A */
  bool a = false;
  /** Key S */
  bool s = false;
  /** Key D */
  bool d = false;

  /** Key SHIFT LEFT */
  bool shift_left = false;
  /** Key SHIFT RIGHT */
  bool shift_right = false;
  /** Key SPACE */
  bool space = false;

  /** Key ESCAPE */
  bool escape = false;
  /** Key CONTROL LEFT */
  bool control_left = false;
  /** Key CONTROL RIGHT */
  bool control_right = false;

  /** Mouse LEFT CLICK */
  bool mouse_left_clicked_ = false;
  /** Mouse RIGHT CLICK */
  bool mouse_right_clicked_ = false;

  /** Stored previous mouse position on X axis */
  double previous_mouse_x = false;
  /** Stored previous mouse position on Y axis */
  double previous_mouse_y = false;

  /** Mouse position on X axis */
  double mouse_x = false;
  /** Mouse position on Y axis */
  double mouse_y = false;

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

/**
 * @brief Struct that represents a RGBA color
 */
struct ClearColor {

  /** Red value */
  float r;
  /** Green value */
  float g;
  /** Blue value */
  float b;
  /** Alpha value */
  float a;

  ClearColor() : r(0.0f), g(0.0f), b(0.0f), a(1.0f){}
  ClearColor(float red, float green, float blue, float alpha){
    r = red;
    g = green; 
    b = blue;
    a = alpha;
  }
};

/**
 * @brief Manages all the functionalities related to the Window, its creation, destruction, detection of events and render of the scenes
 */
class Window {
public:
  //static Window* create(int w, int h, const char* title);
  //static std::optional<Window> create(int width, int height, const char*title, ClearColor color = ClearColor());
  
  /**
   * @brief Creates a window for pc
   * 
   * @param width Width of the window
   * @param height Height of the window
   * @param title Title of the window
   * @param color Background color
   * 
   * @return std::unique_ptr<Window> A pointer to the created window
   */
  static std::unique_ptr<Window> create_opengl(int width, int height, const char* title, ClearColor color = ClearColor());
  
  /**
  * @brief Creates a window for xbox
  *
  * @param width Width of the window
  * @param height Height of the window
  * @param title Title of the window
  * @param color Background color
  *
  * @return std::unique_ptr<Window> A pointer to the created window
  */
  static std::unique_ptr<Window> create_directx11(int width, int height, const char* title, ClearColor color = ClearColor());


  /**
   * @brief Checks if the window is open
   * 
   * @return True when the window is opened, False if not.
   */
  bool is_open() const;

  /**
   * @brief Renders all the elements of the scene
   */
  void render();

  /**
   * @brief Renders all the elements of the scene for the xbox window
   * 
   */
  #ifdef RENDER_DIRECTX11
  void render_xbox(ID3D11DeviceContext* device_cont, ID3D11RenderTargetView* render_target, IDXGISwapChain* swap_c, ID3D11DepthStencilView* depthStencilView, bool is_uwp_ = false);
  #endif
  /**
   * @brief Detects all the events that takes place inside de window
   */
  void detect_events();

  /**
   * @brief Returns the last input that takes place inside of the window
   * 
   * @return Input The last input of that the window detected 
   */
  Input return_last_input();

  /**
   * @brief Returns the delta time
   * 
   * @return double Delta time, the elapsed time between one frame and another 
   */
  double get_delta_time();

  /**
   * @brief Clears the window to draw another frame
   */
  void clear();

  /**
   * @brief Clears the window to draw another frame
   */
  #ifdef RENDER_DIRECTX11
  void clear(ID3D11DeviceContext* device_cont, ID3D11RenderTargetView* renderTargetView, 
    ID3D11DepthStencilView* stencilView);

  #endif

  void reset_input_joystick();
  bool retrieve_input_joystick();

  /**
   * @brief Closes the window
   */
  void close_window();

  /**
   * @brief Sets a title to the window
   * 
   * @param name Given name of the window
   */
  void set_title(const char* name);

  /**
   * @brief Returns the refresh rate of the primary monitor
   *
   * @return int Refresh rate, or -1 if error
   */
  int get_refresh_rate();

  /**
   * @brief Updates the logic to calculate time difference
   * 
   */
  void update_delta();

  /**
   * @brief If has passed enough time to tick the update according to frame limit
   * 
   */
  bool tick();

  /**
   * @brief Change the FPS limit of the window to the most approximate one with your primary monitor refresh rate
   * 
   * @param new_fps_limit The new FPS limit if different than 0. If zero, set to refresh_rate
   */
  void set_fps_limit(unsigned int new_fps_limit);

  ~Window();
  Window(Window& right);
  Window(Window&& right);

  /** Pointer to a GLFWwindow object */
  #ifdef RENDER_OPENGL
  GLFWwindow* window;
  #endif
  #ifdef RENDER_DIRECTX11
  HWND window;

  #endif
  /** Window background color */
  ClearColor clear_color_;
  /** Window width */
  int width_;
  /** Window height */
  int height_;
  /** Refresh rate */
  int refresh_rate_;
  /** Time it needs to pass to adjust to framerate */
  double fps_step;


  double delta_time_ = 0.0;


#ifdef RENDER_DIRECTX11
  bool keys[256] = { false };
  bool marked_for_close_ = false;


  Window() {};
#endif


  bool has_joystick_ = false;
  XINPUT_STATE joystick_state_;

private:

  /**
   * @brief Reads the input from the window
   */
  void store_input();

  /** Input of the window */
  Input input_;
  //clock_t last_time_;

  double last_time_;
  double now_time_;
  double accumulated_delta_ = 0.0;


  /** If a tick has just produced based on fps_step and accumulated delta */
  bool tick_;
  #ifdef RENDER_OPENGL
    Window(GLFWwindow* wh, int w, int h, int refresh);
  #endif
  Window(const Window&) = delete;
};

#endif //__WINDOW_HPP__