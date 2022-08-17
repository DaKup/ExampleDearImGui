//#include <iostream>
//#include <string>
//
//#include "lib.hpp"
//
// auto main() -> int
//{
//  auto const lib = library {};
//  auto const message = "Hello from " + lib.name + "!";
//  std::cout << message << '\n';
//  return 0;
//}

extern void gui_application();

// Dear ImGui: standalone example application for Glfw + Vulkan
// If you are new to Dear ImGui, read documentation from the docs/ folder + read
// the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h
// in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface
// with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your
//   engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this
// example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used
//   by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

static void check_vk_result(VkResult err)
{
    if (err == 0)
    {
        return;
    }

    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

    if (err < 0)
    {
        abort();
    }
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object, size_t location, int32_t messageCode,
                                                   const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

class Application
{
  public:
    Application();
    virtual ~Application();

    int SetupApplication();
    void RunMainLoop();

  private:
    void SetupVulkan(const char **extensions, uint32_t extensions_count);
    void SetupVulkanWindow(VkSurfaceKHR surface, int width, int height);
    void CleanupVulkanWindow();
    void CleanupVulkan();

    void FrameRender(ImDrawData *draw_data);
    void FramePresent();

    // glfw
    GLFWwindow *m_glfw_window{nullptr};

    // vulkan
    VkAllocationCallbacks *m_vk_allocation_callbacks{nullptr};
    VkInstance m_vk_instance{nullptr};
    VkPhysicalDevice m_vk_physical_device{nullptr};
    VkDevice m_vk_device{nullptr};
    uint32_t m_nQueueFamily{(uint32_t)-1};
    VkQueue m_vk_queue{nullptr};
    VkDebugReportCallbackEXT m_vk_debug_report_callback_ext{nullptr};
    VkPipelineCache m_vk_pipeline_cache{nullptr};
    VkDescriptorPool m_vk_descriptor_pool{nullptr};

    // imgui
    ImGui_ImplVulkanH_Window m_imgui_vulkan_window;
    int m_nMinImageCount;
    bool m_bSwapChainRebuild;
};

auto main(int argc, char *argv[]) -> int
{
    Application app;
    app.SetupApplication();
    app.RunMainLoop();
    return 0;
}

Application::Application()
{
    m_glfw_window = nullptr;
    m_vk_allocation_callbacks = nullptr;
    VkInstance g_Instance = nullptr;
    VkPhysicalDevice g_PhysicalDevice = nullptr;
    VkDevice g_Device = nullptr;
    uint32_t g_QueueFamily = (uint32_t)-1;
    VkQueue g_Queue = nullptr;
    VkDebugReportCallbackEXT g_DebugReport = nullptr;
    VkPipelineCache g_PipelineCache = nullptr;
    VkDescriptorPool g_DescriptorPool = nullptr;
    ImGui_ImplVulkanH_Window g_MainWindowData;
    m_nMinImageCount = 2;
    m_bSwapChainRebuild = false;
}

Application::~Application()
{
    VkResult err = vkDeviceWaitIdle(m_vk_device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    CleanupVulkanWindow();
    CleanupVulkan();
    glfwDestroyWindow(m_glfw_window);
    glfwTerminate();
}

void Application::RunMainLoop()
{
    // Our state
    //bool show_demo_window = true;
    //bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
    while (!glfwWindowShouldClose(m_glfw_window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
        // your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application, or clear/overwrite your copy of the
        // keyboard data. Generally you may always pass all inputs to dear imgui,
        // and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        if (m_bSwapChainRebuild)
        {
            int width, height;
            glfwGetFramebufferSize(m_glfw_window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(m_nMinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(m_vk_instance, m_vk_physical_device, m_vk_device,
                                                       &m_imgui_vulkan_window, m_nQueueFamily,
                                                       m_vk_allocation_callbacks, width, height, m_nMinImageCount);
                m_imgui_vulkan_window.FrameIndex = 0;
                m_bSwapChainRebuild = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        gui_application();

        //// 1. Show the big demo window (Most of the sample code is in
        //// ImGui::ShowDemoWindow()! You can browse its code to learn more about
        /// Dear / ImGui!).
        // if (show_demo_window)
        //   ImGui::ShowDemoWindow(&show_demo_window);

        //// 2. Show a simple window that we create ourselves. We use a Begin/End
        /// pair / to created a named window.
        //{
        //  static float f = 0.0f;
        //  static int counter = 0;

        //  ImGui::Begin("Hello, world!");  // Create a window called "Hello,
        //  world!"
        //                                  // and append into it.

        //  ImGui::Text("This is some useful text.");  // Display some text (you
        //  can
        //                                             // use a format strings
        //                                             too)
        //  ImGui::Checkbox(
        //      "Demo Window",
        //      &show_demo_window);  // Edit bools storing our window open/close
        //      state
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
        //       &show_another_window);  // Pass a pointer to our bool variable
        //       (the
        //                               // window will have a closing button that
        //                               will
        //                               // clear the bool when clicked)
        //   ImGui::Text("Hello from another window!");
        //   if (ImGui::Button("Close Me"))
        //     show_another_window = false;
        //   ImGui::End();
        // }

        // Rendering
        ImGui::Render();
        ImDrawData *main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
        m_imgui_vulkan_window.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        m_imgui_vulkan_window.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        m_imgui_vulkan_window.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        m_imgui_vulkan_window.ClearValue.color.float32[3] = clear_color.w;
        if (!main_is_minimized)
        {
            FrameRender(main_draw_data);
        }

        ImGuiIO &io = ImGui::GetIO();
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present Main Platform Window
        if (!main_is_minimized)
        {
            FramePresent();
        }
    }
}

int Application::SetupApplication()
{
    // Setup GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_glfw_window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+Vulkan example", NULL, NULL);

    // Setup Vulkan
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }
    uint32_t extensions_count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    SetupVulkan(extensions, extensions_count);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(m_vk_instance, m_glfw_window, m_vk_allocation_callbacks, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(m_glfw_window, &w, &h);
    SetupVulkanWindow(surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
    // Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_glfw_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_vk_instance;
    init_info.PhysicalDevice = m_vk_physical_device;
    init_info.Device = m_vk_device;
    init_info.QueueFamily = m_nQueueFamily;
    init_info.Queue = m_vk_queue;
    init_info.PipelineCache = m_vk_pipeline_cache;
    init_info.DescriptorPool = m_vk_descriptor_pool;
    init_info.Subpass = 0;
    init_info.MinImageCount = m_nMinImageCount;
    init_info.ImageCount = m_imgui_vulkan_window.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = m_vk_allocation_callbacks;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, m_imgui_vulkan_window.RenderPass);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please
    // handle those errors in your application (e.g. use an assertion, or display
    // an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored
    // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = m_imgui_vulkan_window.Frames[m_imgui_vulkan_window.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = m_imgui_vulkan_window.Frames[m_imgui_vulkan_window.FrameIndex].CommandBuffer;

        err = vkResetCommandPool(m_vk_device, command_pool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        check_vk_result(err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = vkEndCommandBuffer(command_buffer);
        check_vk_result(err);
        err = vkQueueSubmit(m_vk_queue, 1, &end_info, VK_NULL_HANDLE);
        check_vk_result(err);

        err = vkDeviceWaitIdle(m_vk_device);
        check_vk_result(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    return 0;
}

void Application::SetupVulkan(const char **extensions, uint32_t extensions_count)
{
    VkResult err;

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount = extensions_count;
        create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Enabling validation layers
        const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;

        // Enable debug report extension (we need additional storage, so we
        // duplicate the user array to add our new extension to it)
        std::vector<const char *> extensions_ext(extensions_count + 1);
        memcpy(&extensions_ext[0], extensions, extensions_count * sizeof(const char *));

        extensions_ext[extensions_count] = "VK_EXT_debug_report";
        create_info.enabledExtensionCount = extensions_count + 1;
        create_info.ppEnabledExtensionNames = &extensions_ext[0];

        // Create Vulkan Instance
        err = vkCreateInstance(&create_info, m_vk_allocation_callbacks, &m_vk_instance);
        check_vk_result(err);

        // Get the function pointer (required for any extensions)
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vk_instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = NULL;
        err = vkCreateDebugReportCallbackEXT(m_vk_instance, &debug_report_ci, m_vk_allocation_callbacks,
                                             &m_vk_debug_report_callback_ext);
        check_vk_result(err);
#else
        // Create Vulkan Instance without any debug feature
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
        IM_UNUSED(g_DebugReport);
#endif
    }

    // Select GPU
    {
        uint32_t gpu_count;
        err = vkEnumeratePhysicalDevices(m_vk_instance, &gpu_count, NULL);
        check_vk_result(err);
        IM_ASSERT(gpu_count > 0);

        std::vector<VkPhysicalDevice> gpus(gpu_count);
        err = vkEnumeratePhysicalDevices(m_vk_instance, &gpu_count, &gpus[0]);
        check_vk_result(err);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use
        // first one available. This covers most common cases
        // (multi-gpu/integrated+dedicated graphics). Handling more complicated
        // setups (multiple dedicated GPUs) is out of scope of this sample.
        uint32_t use_gpu = 0;
        for (uint32_t i = 0; i < gpu_count; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                use_gpu = i;
                break;
            }
        }

        m_vk_physical_device = gpus[use_gpu];
    }

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &count, NULL);
        std::vector<VkQueueFamilyProperties> queues(count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &count, &queues[0]);
        for (uint32_t i = 0; i < count; i++)
        {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_nQueueFamily = i;
                break;
            }
        }
        IM_ASSERT(m_nQueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
        int device_extension_count = 1;
        const char *device_extensions[] = {"VK_KHR_swapchain"};
        const float queue_priority[] = {1.0f};
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = m_nQueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = device_extension_count;
        create_info.ppEnabledExtensionNames = device_extensions;
        err = vkCreateDevice(m_vk_physical_device, &create_info, m_vk_allocation_callbacks, &m_vk_device);
        check_vk_result(err);
        vkGetDeviceQueue(m_vk_device, m_nQueueFamily, 0, &m_vk_queue);
    }

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(m_vk_device, &pool_info, m_vk_allocation_callbacks, &m_vk_descriptor_pool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used
// by the demo. Your real engine/app may not use them.
void Application::SetupVulkanWindow(VkSurfaceKHR surface, int width, int height)
{
    m_imgui_vulkan_window.Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_vk_physical_device, m_nQueueFamily, m_imgui_vulkan_window.Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    m_imgui_vulkan_window.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        m_vk_physical_device, m_imgui_vulkan_window.Surface, requestSurfaceImageFormat,
        (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                        VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    m_imgui_vulkan_window.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        m_vk_physical_device, m_imgui_vulkan_window.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(m_nMinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(m_vk_instance, m_vk_physical_device, m_vk_device, &m_imgui_vulkan_window,
                                           m_nQueueFamily, m_vk_allocation_callbacks, width, height, m_nMinImageCount);
}

void Application::CleanupVulkan()
{
    vkDestroyDescriptorPool(m_vk_device, m_vk_descriptor_pool, m_vk_allocation_callbacks);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vk_instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(m_vk_instance, m_vk_debug_report_callback_ext, m_vk_allocation_callbacks);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(m_vk_device, m_vk_allocation_callbacks);
    vkDestroyInstance(m_vk_instance, m_vk_allocation_callbacks);
}

void Application::CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(m_vk_instance, m_vk_device, &m_imgui_vulkan_window, m_vk_allocation_callbacks);
}

void Application::FrameRender(ImDrawData *draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore =
        m_imgui_vulkan_window.FrameSemaphores[m_imgui_vulkan_window.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore =
        m_imgui_vulkan_window.FrameSemaphores[m_imgui_vulkan_window.SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(m_vk_device, m_imgui_vulkan_window.Swapchain, UINT64_MAX, image_acquired_semaphore,
                                VK_NULL_HANDLE, &m_imgui_vulkan_window.FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        m_bSwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame *fd = &m_imgui_vulkan_window.Frames[m_imgui_vulkan_window.FrameIndex];
    {
        err = vkWaitForFences(m_vk_device, 1, &fd->Fence, VK_TRUE,
                              UINT64_MAX); // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(m_vk_device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(m_vk_device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_imgui_vulkan_window.RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = m_imgui_vulkan_window.Width;
        info.renderArea.extent.height = m_imgui_vulkan_window.Height;
        info.clearValueCount = 1;
        info.pClearValues = &m_imgui_vulkan_window.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(m_vk_queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

void Application::FramePresent()
{
    if (m_bSwapChainRebuild)
    {
        return;
    }
    VkSemaphore render_complete_semaphore =
        m_imgui_vulkan_window.FrameSemaphores[m_imgui_vulkan_window.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &m_imgui_vulkan_window.Swapchain;
    info.pImageIndices = &m_imgui_vulkan_window.FrameIndex;
    VkResult err = vkQueuePresentKHR(m_vk_queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        m_bSwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    m_imgui_vulkan_window.SemaphoreIndex =
        (m_imgui_vulkan_window.SemaphoreIndex + 1) % m_imgui_vulkan_window.ImageCount; // Now we can use the next set of
                                                                                       // semaphores
}
