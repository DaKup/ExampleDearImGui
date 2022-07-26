#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>



// Our state
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


void gui_application()
{

    ImGuiIO& io = ImGui::GetIO();
  //ImGui::TextWrapped(
  //    "Below we are displaying the font texture (which is the only texture we "
  //    "have access to in this demo). "
  //    "Use the 'ImTextureID' type as storage to pass pointers or identifier to "
  //    "your own texture data. "
  //    "Hover the texture for a zoomed view!");

  // Below we are displaying the font texture because it is the only texture we
  // have access to inside the demo! Remember that ImTextureID is just storage
  // for whatever you want it to be. It is essentially a value that will be
  // passed to the rendering backend via the ImDrawCmd structure. If you use one
  // of the default imgui_impl_XXXX.cpp rendering backend, they all have
  // comments at the top of their respective source file to specify what they
  // expect to be stored in ImTextureID, for example:
  // - The imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*'
  // pointer
  // - The imgui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture
  // identifier, etc. More:
  // - If you decided that ImTextureID = MyEngineTexture*, then you can pass
  // your MyEngineTexture* pointers
  //   to ImGui::Image(), and gather width/height through your own functions,
  //   etc.
  // - You can use ShowMetricsWindow() to inspect the draw data that are being
  // passed to your renderer,
  //   it will help you debug issues if you are confused about it.
  // - Consider using the lower-level ImDrawList::AddImage() API, via
  // ImGui::GetWindowDrawList()->AddImage().
  // - Read https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
  // - Read
  // https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples

  ImTextureID my_tex_id = io.Fonts->TexID;
  float my_tex_w = (float)io.Fonts->TexWidth;
  float my_tex_h = (float)io.Fonts->TexHeight;
  
  {
    //ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
    //ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // No tint
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);  // 50% opaque white

    ImGui::Image(my_tex_id,
                 ImVec2(my_tex_w, my_tex_h),
                 uv_min,
                 uv_max,
                 tint_col,
                 border_col);

    //if (ImGui::IsItemHovered()) {
    //  ImGui::BeginTooltip();
    //  float region_sz = 32.0f;
    //  float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
    //  float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
    //  float zoom = 4.0f;
    //  if (region_x < 0.0f) {
    //    region_x = 0.0f;
    //  } else if (region_x > my_tex_w - region_sz) {
    //    region_x = my_tex_w - region_sz;
    //  }
    //  if (region_y < 0.0f) {
    //    region_y = 0.0f;
    //  } else if (region_y > my_tex_h - region_sz) {
    //    region_y = my_tex_h - region_sz;
    //  }
    //  ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
    //  ImGui::Text(
    //      "Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
    //  ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
    //  ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w,
    //                      (region_y + region_sz) / my_tex_h);
    //  ImGui::Image(my_tex_id,
    //               ImVec2(region_sz * zoom, region_sz * zoom),
    //               uv0,
    //               uv1,
    //               tint_col,
    //               border_col);
    //  ImGui::EndTooltip();
    //}
  }


  //// 1. Show the big demo window (Most of the sample code is in
  //// ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
  //// ImGui!).
  // if (show_demo_window)
  //   ImGui::ShowDemoWindow(&show_demo_window);

  //// 2. Show a simple window that we create ourselves. We use a Begin/End pair
  //// to created a named window.
  //{
  //  static float f = 0.0f;
  //  static int counter = 0;

  //  ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
  //                                  // and append into it.

  //  ImGui::Text("This is some useful text.");  // Display some text (you can
  //                                             // use a format strings too)
  //  ImGui::Checkbox(
  //      "Demo Window",
  //      &show_demo_window);  // Edit bools storing our window open/close state
  //  ImGui::Checkbox("Another Window", &show_another_window);

  //  ImGui::SliderFloat(
  //      "float",
  //      &f,
  //      0.0f,
  //      1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
  //  ImGui::ColorEdit3(
  //      "clear color",
  //      (float*)&clear_color);  // Edit 3 floats representing a color

  //  if (ImGui::Button(
  //          "Button"))  // Buttons return true when clicked (most widgets
  //                      // return true when edited/activated)
  //    counter++;
  //  ImGui::SameLine();
  //  ImGui::Text("counter = %d", counter);

  //  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
  //              1000.0f / ImGui::GetIO().Framerate,
  //              ImGui::GetIO().Framerate);
  //  ImGui::End();
  //}

  //// 3. Show another simple window.
  // if (show_another_window) {
  //   ImGui::Begin(
  //       "Another Window",
  //       &show_another_window);  // Pass a pointer to our bool variable (the
  //                               // window will have a closing button that will
  //                               // clear the bool when clicked)
  //   ImGui::Text("Hello from another window!");
  //   if (ImGui::Button("Close Me"))
  //     show_another_window = false;
  //   ImGui::End();
  // }
}
