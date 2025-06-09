#include "window.hpp"

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


#ifdef RENDER_DIRECTX11
double GetTimeAsDouble() {
  using namespace std::chrono;
  using SecondsFP = std::chrono::duration<double>;
  return duration_cast<SecondsFP>(high_resolution_clock::now().time_since_epoch()).count();
}
#endif

//std::optional<Window> Window::create(int width, int height, const char* title, ClearColor color) {
std::unique_ptr<Window> Window::create_opengl(int width, int height, const char* title, ClearColor color) {
  #ifdef RENDER_OPENGL
  //Create hints to stablish the window behavior
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


  //Create the GLFWwindow and exit if it fails
  GLFWwindow* win_handle_ = glfwCreateWindow(width, height, title, NULL, NULL);
  
  if (!win_handle_) {return nullptr;}

  //Mark the window as the current context
  glfwMakeContextCurrent(win_handle_);

  //Sets the number of frames waiting to swap the render buffers
  glfwSwapInterval(1);

  //Stablish the minimum and maximum values of the viewport
  glViewport(0, 0, width, height);

  Window window {win_handle_, width, height,glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate };

  window.clear_color_ = color;
  window.width_ = width;
  window.height_ = height;
  window.window = win_handle_;

  //Stablish values for the update with limited frames
  window.refresh_rate_ = window.get_refresh_rate();
  return std::make_unique<Window>(window);
  #endif
  return nullptr;
}

#ifdef RENDER_DIRECTX11
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::unique_ptr<Window> Window::create_directx11(int width, int height, const char* title, ClearColor color) {
  // Register the window class.
  const wchar_t CLASS_NAME[] = L"EVE_WindowDirectX11";

  WNDCLASS wc = { };
  HINSTANCE inst=GetModuleHandle(NULL);

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = inst;
  wc.lpszClassName = CLASS_NAME;
  

  RegisterClass(&wc);

  //This is needed to adjust the client size to the window
  RECT wr = { 0, 0, width, height };    // set the size, but not the position
  AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

  HWND win = CreateWindowEx(NULL,
    CLASS_NAME,    // name of the window class
    (LPCWSTR)title,   // title of the window
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,    // window style
    50,    // x-position of the window
    50,    // y-position of the window
    wr.right - wr.left,    // width of the window
    wr.bottom - wr.top,
    NULL,    // we have no parent window, NULL
    NULL,    // we aren't using menus, NULL
    inst,    // application handle
    NULL);    // used with multiple windows, NULL

  Window window;

  window.clear_color_ = color;
  window.width_ = width;
  window.height_ = height;
  window.window = win;
  window.marked_for_close_ = false;

  // display the window on the screen
  ShowWindow(win, SW_SHOW);

  //Stablish values for the update with limited frames
  window.refresh_rate_ = window.get_refresh_rate();

  return std::make_unique<Window>(window);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
  if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))return true;

    //printf("WindowProc; %#010x\n", uMsg);
    switch (uMsg)  {
      case WM_CLOSE:{
        DestroyWindow(hwnd);
        PostQuitMessage(0);
      }return 0;
      case WM_DESTROY:{
        PostQuitMessage(0);
      }return 0;
      case WM_WINDOWPOSCHANGING: {

      }return 0;
      case WM_LBUTTONDOWN:{
      }return 0;
      case WM_LBUTTONUP:{
      }return 0;
      case WM_PAINT: {
          PAINTSTRUCT ps;
          HDC hdc = BeginPaint(hwnd, &ps);

          // All painting occurs here, between BeginPaint and EndPaint.

          FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

          EndPaint(hwnd, &ps);
      }return 0;
    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

double Window::get_delta_time() {return delta_time_;}

/**
* Render image to the window
*/
#ifdef RENDER_OPENGL
void Window::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}
#endif


#ifdef RENDER_DIRECTX11
void Window::render_xbox(ID3D11DeviceContext* device_cont, ID3D11RenderTargetView* render_target, IDXGISwapChain* swap_c, ID3D11DepthStencilView* depthStencilView, bool is_uwp_){
  //*/
  /**/

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  if (!is_uwp_) {
      swap_c->Present(1, 0);
      device_cont->OMSetRenderTargets(1, &render_target, depthStencilView);
  }
}
#endif

#ifdef RENDER_OPENGL
void Window::clear() {
  //Init new imgui frame
  glClearColor(clear_color_.r, clear_color_.g, clear_color_.b, clear_color_.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}
#endif

#ifdef RENDER_DIRECTX11
void Window::clear(ID3D11DeviceContext* device_cont, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* stencilView) {

  // Clear Render Target
  const float clear_color_with_alpha[4] = {
      clear_color_.r * clear_color_.a, clear_color_.g * clear_color_.a,
      clear_color_.b * clear_color_.a, clear_color_.a
  };
  // Clear Z-Buffer
  device_cont->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  device_cont->ClearRenderTargetView(renderTargetView, clear_color_with_alpha);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}


#endif


void Window::reset_input_joystick() {
    //Buttons
    input_.button_x = false;
    input_.button_y = false;
    input_.button_a = false;
    input_.button_b = false;
    input_.button_rb = false;
    input_.button_lb = false;

    //D-pad
    input_.dpad_up = false;
    input_.dpad_down = false;
    input_.dpad_left = false;
    input_.dpad_right = false;

    //Specials
    input_.start = false;
    input_.select = false;

    //Trigger pressed or not, and force
    input_.trigger_r = false;
    input_.trigger_l = false;
    input_.trigger_r_force = 0;
    input_.trigger_l_force = 0;

    //Sticks
    input_.left_thumb = false;
    input_.right_thumb = false;
    input_.l_stick_x = 0.0f;
    input_.l_stick_y = 0.0f;
    input_.l_stick_norm_x = 0.0f;
    input_.l_stick_norm_y = 0.0f;

    input_.r_stick_x = 0.0f;
    input_.r_stick_y = 0.0f;
    input_.r_stick_norm_x = 0.0f;
    input_.r_stick_norm_y = 0.0f;
}

bool Window::retrieve_input_joystick() {

    //Reset state
    ZeroMemory(&joystick_state_, sizeof(XINPUT_STATE));

    DWORD dwResult = 0;
    dwResult = XInputGetState(0, &joystick_state_);
    if (dwResult != ERROR_SUCCESS) {
        //Controller disconected
        return false;
    }

    reset_input_joystick();

    //Controller connected, map input

    XINPUT_GAMEPAD gPad = joystick_state_.Gamepad;

    //--LEFT STICK
    input_.l_stick_x = gPad.sThumbLX;
    input_.l_stick_y = gPad.sThumbLY;

    //determine how far the controller is pushed
    float magnitude = (float)sqrt(input_.l_stick_x * input_.l_stick_x + input_.l_stick_y * input_.l_stick_y);

    //determine the direction the controller is pushed
    input_.l_stick_norm_x = input_.l_stick_x / magnitude;
    input_.l_stick_norm_y = input_.l_stick_y / magnitude;

    //--RIGHT STICK
    input_.r_stick_x = gPad.sThumbRX;
    input_.r_stick_y = gPad.sThumbRY;

    //determine how far the controller is pushed
    magnitude = (float)sqrt(input_.r_stick_x * input_.r_stick_x + input_.r_stick_y * input_.r_stick_y);

    //determine the direction the controller is pushed
    input_.r_stick_norm_x = input_.r_stick_x / magnitude;
    input_.r_stick_norm_y = input_.r_stick_y / magnitude;

    /*
    WORD                                wButtons;
    */

    //Triggers
    input_.trigger_r_force = gPad.bRightTrigger;
    if (gPad.bRightTrigger == 0) { input_.trigger_r = false; }

    input_.trigger_l_force = gPad.bLeftTrigger;
    if (gPad.bLeftTrigger == 0) { input_.trigger_l = false; }

    //Buttons
    input_.button_x = (gPad.wButtons & XINPUT_GAMEPAD_X);
    input_.button_y = (gPad.wButtons & XINPUT_GAMEPAD_Y);
    input_.button_b = (gPad.wButtons & XINPUT_GAMEPAD_B);
    input_.button_a = (gPad.wButtons & XINPUT_GAMEPAD_A);
    input_.button_rb = (gPad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
    input_.button_lb = (gPad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
    input_.start = (gPad.wButtons & XINPUT_GAMEPAD_START);
    input_.select = (gPad.wButtons & XINPUT_GAMEPAD_BACK);

    input_.dpad_up = (gPad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
    input_.dpad_down = (gPad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
    input_.dpad_left = (gPad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
    input_.dpad_right = (gPad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

    input_.left_thumb = (gPad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
    input_.right_thumb = (gPad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);


    return true;

}

void Window::close_window(){
  #ifdef RENDER_OPENGL
  glfwSetWindowShouldClose(window, GL_TRUE);
  #endif 
  #ifdef RENDER_DIRECTX11
  marked_for_close_ = true;
  #endif
}

int Window::get_refresh_rate(){
  #ifdef RENDER_OPENGL
  GLFWmonitor* m = glfwGetPrimaryMonitor();
  if(m!=nullptr){return glfwGetVideoMode(m)->refreshRate;}
  #endif
  
  #ifdef RENDER_DIRECTX11
  DEVMODE devMode;
  devMode.dmSize = sizeof(DEVMODE) * 10;
  devMode.dmDriverExtra = 0;
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
  return devMode.dmDisplayFrequency;
  #endif

  return -1;
}

void Window::set_title(const char* name){
#ifdef RENDER_OPENGL
  glfwSetWindowTitle(window, name);
#endif
#ifdef RENDER_DIRECTX11
  SetWindowTextA(window, (LPCSTR)name);
#endif
}


void Window::detect_events() {
  #ifdef RENDER_OPENGL
  glfwPollEvents();
  #endif
  store_input();
  #ifdef RENDER_DIRECTX11
  #endif
  retrieve_input_joystick();
}
void Window::store_input() {
  //One of GLFW_PRESS or GLFW_RELEASE
  
  #ifdef RENDER_OPENGL
  int t;
  //Up
  t= glfwGetKey(window, GLFW_KEY_UP);
  input_.up = (t==GLFW_PRESS);

  //Down
  t = glfwGetKey(window, GLFW_KEY_DOWN);
  input_.down = (t == GLFW_PRESS);

  //Right
  t = glfwGetKey(window, GLFW_KEY_RIGHT);
  input_.right = (t == GLFW_PRESS);

  //Left
  t = glfwGetKey(window, GLFW_KEY_LEFT);
  input_.left = (t == GLFW_PRESS);

  //W
  t = glfwGetKey(window, GLFW_KEY_W);
  input_.w = (t == GLFW_PRESS);
  //A
  t = glfwGetKey(window, GLFW_KEY_A);
  input_.a = (t == GLFW_PRESS);
  //S
  t = glfwGetKey(window, GLFW_KEY_S);
  input_.s = (t == GLFW_PRESS);
  //D
  t = glfwGetKey(window, GLFW_KEY_D);
  input_.d = (t == GLFW_PRESS);

  //Shift left
  t = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
  input_.shift_left = (t == GLFW_PRESS);

  //Shift right
  t = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
  input_.shift_right = (t == GLFW_PRESS);

  //Control left
  t = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
  input_.control_left = (t == GLFW_PRESS);

  //Control right
  t = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
  input_.control_right = (t == GLFW_PRESS);

   //Space
  t = glfwGetKey(window, GLFW_KEY_SPACE);
  input_.space = (t == GLFW_PRESS);

  //Escape
  t = glfwGetKey(window, GLFW_KEY_ESCAPE);
  input_.escape = (t == GLFW_PRESS);
  

  //Store previous mouse position to detect dragging
  input_.previous_mouse_x = input_.mouse_x;
  input_.previous_mouse_y = input_.mouse_y;

  //Mouse position
  glfwGetCursorPos(window, &input_.mouse_x, &input_.mouse_y);

  //Mouse click left and right
  t = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  input_.mouse_left_clicked_ = (t == GLFW_PRESS);

  t = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
  
  input_.mouse_right_clicked_ = (t == GLFW_PRESS);
#endif
#ifdef RENDER_DIRECTX11
  //Normal keys
  input_.a = keys['A'];
  input_.w = keys['W'];
  input_.s = keys['S'];
  input_.d = keys['D'];


  //Mouse position
  POINT p;
  if (GetCursorPos(&p) && ScreenToClient(window, &p)) {
    input_.previous_mouse_x = input_.mouse_x;
    input_.previous_mouse_y = input_.mouse_y;
    input_.mouse_x = p.x;
    input_.mouse_y = p.y;
  }


#endif
}

Input Window::return_last_input() {return input_;}

bool Window::is_open() const {
  #ifdef RENDER_OPENGL
  return !glfwWindowShouldClose(window);
  #endif
  #ifdef RENDER_DIRECTX11
  return (marked_for_close_ == false);
  #endif
}

#ifdef RENDER_OPENGL
Window::Window(GLFWwindow* wh, int w, int h, int refresh): window{wh},width_{w}, height_{h}, refresh_rate_ { refresh } {
  last_time_ = glfwGetTime();
  fps_step = 1.0 / refresh_rate_;
}
#endif

Window::Window(Window& right) : window{ right.window } {
  width_ = right.width_;
  height_ = right.height_;
  clear_color_ = right.clear_color_;
  refresh_rate_ = right.refresh_rate_;

#ifdef RENDER_OPENGL
  last_time_ = glfwGetTime();
#endif

#ifdef RENDER_DIRECTX11
  last_time_ = GetTimeAsDouble();
  marked_for_close_ = right.marked_for_close_;
#endif
  fps_step = 1.0 / refresh_rate_;

  right.window = 0;
}

Window::Window(Window&& right) : window{ right.window } {
  width_ = right.width_;
  height_ = right.height_;
  clear_color_ = right.clear_color_;
  refresh_rate_ = right.refresh_rate_;

  //&last_time_ = glfwGetTime();
#ifdef RENDER_OPENGL
  last_time_ = glfwGetTime();
#endif

#ifdef RENDER_DIRECTX11
  last_time_ = GetTimeAsDouble();
  marked_for_close_ = right.marked_for_close_;
#endif

  fps_step = 1.0 / refresh_rate_;

  right.window = 0;
}



void Window::update_delta(){
  //Get time difference
#ifdef RENDER_OPENGL
  now_time_ = glfwGetTime();
#endif

#ifdef RENDER_DIRECTX11

  //Identify events
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    // translate keystroke messages into the right format
    TranslateMessage(&msg);

    //Process based on the input
    if (msg.message == WM_KEYDOWN) {
      switch(msg.wParam){
        case VK_SPACE: input_.space = true;break;
        case VK_ESCAPE: input_.escape = true;break;
        case VK_SHIFT: input_.shift_left= true;break;
        case VK_UP: input_.up = true;break;
        case VK_DOWN: input_.down = true;break;
        case VK_RIGHT: input_.right = true;break;
        case VK_LEFT: input_.left = true;break;
        case VK_LCONTROL: input_.control_left = true;break;
        case VK_RCONTROL: input_.control_right = true;break;
        default:keys[msg.wParam] = true;break;
      }
    }
    else if(msg.message == WM_KEYUP){
      switch (msg.wParam) {
      case VK_SPACE: input_.space = false; break;
      case VK_ESCAPE: input_.escape = false; break;
      case VK_SHIFT: input_.shift_left = false; break;
      case VK_UP: input_.up = false; break;
      case VK_DOWN: input_.down = false; break;
      case VK_RIGHT: input_.right = false; break;
      case VK_LEFT: input_.left = false; break;
      case VK_LCONTROL: input_.control_left = false; break;
      case VK_RCONTROL: input_.control_right = false; break;
      default:keys[msg.wParam] = false; break;
      }
    }
    else if(msg.message == WM_LBUTTONDOWN){
      // send the message to the WindowProc function
      DispatchMessage(&msg);
      input_.mouse_left_clicked_ = true;
    }
    else if(msg.message == WM_LBUTTONUP){
      // send the message to the WindowProc function
      DispatchMessage(&msg);
      input_.mouse_left_clicked_ = false;
    }
    else if (msg.message == WM_RBUTTONDOWN) { input_.mouse_right_clicked_ = true; }
    else if (msg.message == WM_RBUTTONUP) { input_.mouse_right_clicked_ = false; }
    else {
      // send the message to the WindowProc function
      DispatchMessage(&msg);

      if (msg.message == WM_QUIT) {
        close_window();
      }
    }
  }

  now_time_ = GetTimeAsDouble();
#endif

  accumulated_delta_ += (now_time_ - last_time_);
  last_time_ = now_time_;

  //Set tick to okay if has passed more time than the fps_step
  tick_ = false;
  delta_time_ = 0.0;

  if (accumulated_delta_ >= fps_step) {
    tick_ = true;
    delta_time_ = accumulated_delta_;
    accumulated_delta_ -= fps_step;
  }
}

bool Window::tick(){return tick_;}

void Window::set_fps_limit(unsigned int new_fps_limit){
  #ifdef RENDER_OPENGL
  if (new_fps_limit != 0) {
    fps_step = 1.0 / new_fps_limit;
    glfwSwapInterval(glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate/new_fps_limit);
  }
  else {
    fps_step = 1.0 / glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
  }
  #endif
  #ifdef RENDER_DIRECTX11
  //TODO:
  #endif
}

Window::~Window() {
  if (window != 0) {
    #ifdef RENDER_OPENGL
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
    #endif 
    #ifdef RENDER_DIRECTX11
        ImGui_ImplWin32_Shutdown();
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        DestroyWindow(window);
    #endif
  }
}