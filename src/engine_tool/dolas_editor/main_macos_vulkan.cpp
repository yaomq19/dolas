#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <dispatch/dispatch.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#import <AppKit/AppKit.h>

#ifndef DOLAS_EDITOR_RUNTIME_SHADER_DIR
#define DOLAS_EDITOR_RUNTIME_SHADER_DIR "."
#endif

namespace
{
    constexpr int kInitialWidth = 1280;
    constexpr int kInitialHeight = 720;
    constexpr int kMaxFramesInFlight = 2;
    constexpr float kPi = 3.14159265358979323846f;

    struct Matrix4
    {
        float value[16];
    };

    struct PerViewUniform
    {
        Matrix4 view;
        Matrix4 projection;
        float camera_position[4];
    };

    struct PerObjectUniform
    {
        Matrix4 world;
    };

    struct PerFrameUniform
    {
        float light_direction_intensity[4];
        float light_color[4];
    };

    struct GlobalUniform
    {
        float debug_draw_color[4];
    };

    struct SceneVertex
    {
        float position[3];
        float texcoord[2];
        float normal[3];
        float tangent[3];
    };

    struct SceneObject
    {
        std::string name;
        uint32_t first_vertex = 0;
        uint32_t vertex_count = 0;
        std::array<float, 4> color {};
        VkBuffer object_buffer = VK_NULL_HANDLE;
        VkDeviceMemory object_memory = VK_NULL_HANDLE;
        VkBuffer color_buffer = VK_NULL_HANDLE;
        VkDeviceMemory color_memory = VK_NULL_HANDLE;
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    };

    struct RenderAttachment
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    Matrix4 IdentityMatrix()
    {
        Matrix4 matrix {};
        matrix.value[0] = 1.0f;
        matrix.value[5] = 1.0f;
        matrix.value[10] = 1.0f;
        matrix.value[15] = 1.0f;
        return matrix;
    }

    std::vector<char> ReadBinaryFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open binary file: " + path);
        }

        const std::streamsize file_size = file.tellg();
        if (file_size <= 0)
        {
            throw std::runtime_error("Binary file is empty: " + path);
        }

        std::vector<char> buffer(static_cast<size_t>(file_size));
        file.seekg(0);
        file.read(buffer.data(), file_size);
        if (!file)
        {
            throw std::runtime_error("Failed to read binary file: " + path);
        }

        return buffer;
    }

    SceneVertex MakeSceneVertex(float x, float y, float z, float u, float v)
    {
        return {
            { x, y, z },
            { u, v },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f }
        };
    }

    void AppendQuad(std::vector<SceneVertex>& vertices, float x0, float y0, float x1, float y1, float z)
    {
        vertices.push_back(MakeSceneVertex(x0, y0, z, 0.0f, 1.0f));
        vertices.push_back(MakeSceneVertex(x1, y0, z, 1.0f, 1.0f));
        vertices.push_back(MakeSceneVertex(x1, y1, z, 1.0f, 0.0f));
        vertices.push_back(MakeSceneVertex(x0, y0, z, 0.0f, 1.0f));
        vertices.push_back(MakeSceneVertex(x1, y1, z, 1.0f, 0.0f));
        vertices.push_back(MakeSceneVertex(x0, y1, z, 0.0f, 0.0f));
    }

    void AppendDisc(std::vector<SceneVertex>& vertices, float center_x, float center_y, float radius, float z, int segments)
    {
        for (int i = 0; i < segments; ++i)
        {
            const float angle0 = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * kPi;
            const float angle1 = (static_cast<float>(i + 1) / static_cast<float>(segments)) * 2.0f * kPi;
            const float x0 = center_x + std::cos(angle0) * radius;
            const float y0 = center_y + std::sin(angle0) * radius;
            const float x1 = center_x + std::cos(angle1) * radius;
            const float y1 = center_y + std::sin(angle1) * radius;
            vertices.push_back(MakeSceneVertex(center_x, center_y, z, 0.5f, 0.5f));
            vertices.push_back(MakeSceneVertex(x0, y0, z, 0.5f + std::cos(angle0) * 0.5f, 0.5f - std::sin(angle0) * 0.5f));
            vertices.push_back(MakeSceneVertex(x1, y1, z, 0.5f + std::cos(angle1) * 0.5f, 0.5f - std::sin(angle1) * 0.5f));
        }
    }

    void CheckVk(VkResult result, const char* operation)
    {
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(std::string(operation) + " failed with VkResult " + std::to_string(result));
        }
    }

    bool HasInstanceExtension(const char* name)
    {
        uint32_t count = 0;
        CheckVk(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr), "vkEnumerateInstanceExtensionProperties");
        std::vector<VkExtensionProperties> extensions(count);
        CheckVk(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()), "vkEnumerateInstanceExtensionProperties");

        return std::any_of(extensions.begin(), extensions.end(), [name](const VkExtensionProperties& extension) {
            return std::strcmp(extension.extensionName, name) == 0;
        });
    }

    bool HasDeviceExtension(VkPhysicalDevice physical_device, const char* name)
    {
        uint32_t count = 0;
        CheckVk(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr), "vkEnumerateDeviceExtensionProperties");
        std::vector<VkExtensionProperties> extensions(count);
        CheckVk(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data()), "vkEnumerateDeviceExtensionProperties");

        return std::any_of(extensions.begin(), extensions.end(), [name](const VkExtensionProperties& extension) {
            return std::strcmp(extension.extensionName, name) == 0;
        });
    }

    void PrepareCocoaApplicationForGLFW()
    {
        @autoreleasepool
        {
            [NSApplication sharedApplication];
            if (![[NSRunningApplication currentApplication] isFinishedLaunching])
            {
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
                    [NSApp stop:nil];
                    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                                        location:NSZeroPoint
                                                   modifierFlags:0
                                                       timestamp:0
                                                    windowNumber:0
                                                         context:nil
                                                         subtype:0
                                                           data1:0
                                                           data2:0];
                    [NSApp postEvent:event atStart:YES];
                });
            }
        }
    }

    struct QueueFamilies
    {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        bool Complete() const
        {
            return graphics.has_value() && present.has_value();
        }
    };

    class MacOSVulkanEditor
    {
    public:
        void Run()
        {
            InitWindow();
            InitVulkan();
            InitImGui();
            MainLoop();
            Cleanup();
        }

    private:
        GLFWwindow* m_window = nullptr;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphics_queue = VK_NULL_HANDLE;
        VkQueue m_present_queue = VK_NULL_HANDLE;
        uint32_t m_graphics_queue_family = 0;
        uint32_t m_present_queue_family = 0;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkFormat m_swapchain_format = VK_FORMAT_UNDEFINED;
        VkExtent2D m_swapchain_extent {};
        std::vector<VkImage> m_swapchain_images;
        std::vector<VkImageView> m_swapchain_image_views;
        std::vector<VkFramebuffer> m_framebuffers;
        VkImage m_depth_image = VK_NULL_HANDLE;
        VkDeviceMemory m_depth_image_memory = VK_NULL_HANDLE;
        VkImageView m_depth_image_view = VK_NULL_HANDLE;
        VkFormat m_depth_format = VK_FORMAT_UNDEFINED;
        RenderAttachment m_gbuffer_a;
        RenderAttachment m_gbuffer_b;
        RenderAttachment m_gbuffer_c;
        RenderAttachment m_gbuffer_d;
        RenderAttachment m_gbuffer_depth;
        RenderAttachment m_scene_result;
        VkFramebuffer m_gbuffer_framebuffer = VK_NULL_HANDLE;
        VkFramebuffer m_scene_result_framebuffer = VK_NULL_HANDLE;

        VkRenderPass m_render_pass = VK_NULL_HANDLE;
        VkRenderPass m_gbuffer_render_pass = VK_NULL_HANDLE;
        VkRenderPass m_scene_result_render_pass = VK_NULL_HANDLE;
        VkCommandPool m_command_pool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, kMaxFramesInFlight> m_command_buffers {};
        VkDescriptorPool m_imgui_descriptor_pool = VK_NULL_HANDLE;
        VkDescriptorSet m_imgui_scene_result_descriptor = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_scene_descriptor_set_layout = VK_NULL_HANDLE;
        VkPipelineLayout m_scene_pipeline_layout = VK_NULL_HANDLE;
        VkPipeline m_scene_pipeline = VK_NULL_HANDLE;
        VkPipeline m_scene_result_pipeline = VK_NULL_HANDLE;
        VkPipeline m_gbuffer_pipeline = VK_NULL_HANDLE;
        VkPipeline m_deferred_pipeline = VK_NULL_HANDLE;
        VkPipeline m_present_pipeline = VK_NULL_HANDLE;
        VkDescriptorPool m_scene_descriptor_pool = VK_NULL_HANDLE;
        VkDescriptorSet m_deferred_descriptor_set = VK_NULL_HANDLE;
        VkDescriptorSet m_present_descriptor_set = VK_NULL_HANDLE;
        VkBuffer m_scene_vertex_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_scene_vertex_memory = VK_NULL_HANDLE;
        VkBuffer m_fullscreen_vertex_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_fullscreen_vertex_memory = VK_NULL_HANDLE;
        VkBuffer m_per_view_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_per_view_memory = VK_NULL_HANDLE;
        VkBuffer m_per_frame_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_per_frame_memory = VK_NULL_HANDLE;
        RenderAttachment m_fallback_albedo;
        RenderAttachment m_fallback_normal;
        VkSampler m_linear_sampler = VK_NULL_HANDLE;
        std::vector<SceneObject> m_scene_objects;

        std::array<VkSemaphore, kMaxFramesInFlight> m_image_available_semaphores {};
        std::array<VkSemaphore, kMaxFramesInFlight> m_render_finished_semaphores {};
        std::array<VkFence, kMaxFramesInFlight> m_in_flight_fences {};
        size_t m_current_frame = 0;
        int m_selected_object_index = 0;
        bool m_framebuffer_resized = false;
        bool m_imgui_backend_initialized = false;
        bool m_dock_layout_initialized = false;

        static void FramebufferResizeCallback(GLFWwindow* window, int, int)
        {
            auto* app = static_cast<MacOSVulkanEditor*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->m_framebuffer_resized = true;
            }
        }

        void InitWindow()
        {
            std::cerr << "Initializing GLFW..." << std::endl;
            PrepareCocoaApplicationForGLFW();
            glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);
            glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
            if (!glfwInit())
            {
                throw std::runtime_error("glfwInit failed");
            }
            if (!glfwVulkanSupported())
            {
                throw std::runtime_error("GLFW reports Vulkan is not supported. Install Vulkan SDK/MoltenVK and ensure the loader is visible.");
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            std::cerr << "Creating GLFW window..." << std::endl;
            m_window = glfwCreateWindow(kInitialWidth, kInitialHeight, "DolasEditor - Vulkan", nullptr, nullptr);
            if (!m_window)
            {
                throw std::runtime_error("glfwCreateWindow failed");
            }
            glfwSetWindowUserPointer(m_window, this);
            glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
            std::cerr << "GLFW window created." << std::endl;
        }

        void InitVulkan()
        {
            std::cerr << "Creating Vulkan instance..." << std::endl;
            CreateInstance();
            std::cerr << "Creating Vulkan surface..." << std::endl;
            CheckVk(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface), "glfwCreateWindowSurface");
            std::cerr << "Selecting Vulkan physical device..." << std::endl;
            PickPhysicalDevice();
            std::cerr << "Creating Vulkan logical device..." << std::endl;
            CreateDevice();
            std::cerr << "Creating Vulkan swapchain..." << std::endl;
            CreateSwapchain();
            CreateImageViews();
            CreateRenderPass();
            CreateOffscreenRenderPasses();
            CreateDepthResources();
            CreateOffscreenResources();
            CreateFramebuffers();
            CreateCommandPool();
            CreateSceneSamplerAndFallbackTextures();
            CreateSceneResources();
            CreateCommandBuffers();
            CreateSyncObjects();
            std::cerr << "Vulkan core objects created." << std::endl;
        }

        void InitImGui()
        {
            std::cerr << "Initializing ImGui..." << std::endl;
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGui::StyleColorsDark();

            if (!ImGui_ImplGlfw_InitForVulkan(m_window, true))
            {
                throw std::runtime_error("ImGui_ImplGlfw_InitForVulkan failed");
            }

            CreateImGuiDescriptorPool();

            ImGui_ImplVulkan_InitInfo init_info {};
            init_info.ApiVersion = VK_API_VERSION_1_2;
            init_info.Instance = m_instance;
            init_info.PhysicalDevice = m_physical_device;
            init_info.Device = m_device;
            init_info.QueueFamily = m_graphics_queue_family;
            init_info.Queue = m_graphics_queue;
            init_info.DescriptorPool = m_imgui_descriptor_pool;
            init_info.MinImageCount = 2;
            init_info.ImageCount = static_cast<uint32_t>(m_swapchain_images.size());
            init_info.PipelineInfoMain.RenderPass = m_render_pass;
            init_info.PipelineInfoMain.Subpass = 0;
            init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.CheckVkResultFn = [](VkResult result) {
                if (result != VK_SUCCESS)
                {
                    std::cerr << "ImGui Vulkan backend VkResult: " << result << std::endl;
                }
            };

            if (!ImGui_ImplVulkan_Init(&init_info))
            {
                throw std::runtime_error("ImGui_ImplVulkan_Init failed");
            }
            m_imgui_backend_initialized = true;
            RegisterSceneResultImGuiTexture();

            std::cout << "DolasEditor macOS Vulkan initialized: swapchain "
                      << m_swapchain_extent.width << "x" << m_swapchain_extent.height
                      << ", images=" << m_swapchain_images.size() << std::endl;
        }

        void MainLoop()
        {
            while (!glfwWindowShouldClose(m_window))
            {
                glfwPollEvents();
                DrawFrame();
            }

            CheckVk(vkDeviceWaitIdle(m_device), "vkDeviceWaitIdle");
        }

        void CreateInstance()
        {
            VkApplicationInfo app_info {};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = "DolasEditor";
            app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
            app_info.pEngineName = "Dolas";
            app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
            app_info.apiVersion = VK_API_VERSION_1_2;

            uint32_t glfw_extension_count = 0;
            const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
            if (!glfw_extensions || glfw_extension_count == 0)
            {
                throw std::runtime_error("GLFW did not return required Vulkan instance extensions");
            }

            std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
            VkInstanceCreateFlags create_flags = 0;
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
            if (HasInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
            {
                extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
#endif
#ifdef VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
            if (HasInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            {
                extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
#endif

            VkInstanceCreateInfo create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.flags = create_flags;
            create_info.pApplicationInfo = &app_info;
            create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            create_info.ppEnabledExtensionNames = extensions.data();

            VkResult create_result = vkCreateInstance(&create_info, nullptr, &m_instance);
            if (create_result == VK_ERROR_INCOMPATIBLE_DRIVER)
            {
                throw std::runtime_error("vkCreateInstance failed with VK_ERROR_INCOMPATIBLE_DRIVER. On macOS, verify MoltenVK can access a Metal device in this login/session by running 'vulkaninfo --summary'.");
            }
            CheckVk(create_result, "vkCreateInstance");
        }

        QueueFamilies FindQueueFamilies(VkPhysicalDevice physical_device) const
        {
            QueueFamilies families;

            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

            for (uint32_t i = 0; i < queue_family_count; ++i)
            {
                if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    families.graphics = i;
                }

                VkBool32 present_support = VK_FALSE;
                CheckVk(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, m_surface, &present_support), "vkGetPhysicalDeviceSurfaceSupportKHR");
                if (present_support)
                {
                    families.present = i;
                }

                if (families.Complete())
                {
                    break;
                }
            }

            return families;
        }

        bool DeviceSupportsSwapchain(VkPhysicalDevice physical_device) const
        {
            if (!HasDeviceExtension(physical_device, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            {
                return false;
            }

            uint32_t format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, nullptr);
            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &present_mode_count, nullptr);

            return format_count > 0 && present_mode_count > 0;
        }

        void PickPhysicalDevice()
        {
            uint32_t device_count = 0;
            CheckVk(vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr), "vkEnumeratePhysicalDevices");
            if (device_count == 0)
            {
                throw std::runtime_error("No Vulkan physical device was found");
            }

            std::vector<VkPhysicalDevice> devices(device_count);
            CheckVk(vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data()), "vkEnumeratePhysicalDevices");

            for (VkPhysicalDevice device : devices)
            {
                QueueFamilies families = FindQueueFamilies(device);
                if (families.Complete() && DeviceSupportsSwapchain(device))
                {
                    m_physical_device = device;
                    m_graphics_queue_family = *families.graphics;
                    m_present_queue_family = *families.present;

                    VkPhysicalDeviceProperties properties {};
                    vkGetPhysicalDeviceProperties(device, &properties);
                    std::cout << "Selected Vulkan device: " << properties.deviceName << std::endl;
                    return;
                }
            }

            throw std::runtime_error("No Vulkan physical device supports graphics, present, and swapchain");
        }

        void CreateDevice()
        {
            std::set<uint32_t> unique_queue_families = { m_graphics_queue_family, m_present_queue_family };
            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            float queue_priority = 1.0f;
            for (uint32_t queue_family : unique_queue_families)
            {
                VkDeviceQueueCreateInfo queue_create_info {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;
                queue_create_infos.push_back(queue_create_info);
            }

            std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
            if (HasDeviceExtension(m_physical_device, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            {
                device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
            }
#else
            if (HasDeviceExtension(m_physical_device, "VK_KHR_portability_subset"))
            {
                device_extensions.push_back("VK_KHR_portability_subset");
            }
#endif

            VkPhysicalDeviceFeatures device_features {};

            VkDeviceCreateInfo create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
            create_info.pQueueCreateInfos = queue_create_infos.data();
            create_info.pEnabledFeatures = &device_features;
            create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
            create_info.ppEnabledExtensionNames = device_extensions.data();

            CheckVk(vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device), "vkCreateDevice");
            vkGetDeviceQueue(m_device, m_graphics_queue_family, 0, &m_graphics_queue);
            vkGetDeviceQueue(m_device, m_present_queue_family, 0, &m_present_queue);
        }

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const
        {
            for (const VkSurfaceFormatKHR& format : formats)
            {
                if ((format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM) &&
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return format;
                }
            }
            return formats.front();
        }

        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
        {
            if (capabilities.currentExtent.width != UINT32_MAX)
            {
                return capabilities.currentExtent;
            }

            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D extent {
                static_cast<uint32_t>(std::max(width, 1)),
                static_cast<uint32_t>(std::max(height, 1))
            };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return extent;
        }

        void CreateSwapchain()
        {
            VkSurfaceCapabilitiesKHR capabilities {};
            CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

            uint32_t format_count = 0;
            CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR");
            std::vector<VkSurfaceFormatKHR> formats(format_count);
            CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR");

            VkSurfaceFormatKHR surface_format = ChooseSurfaceFormat(formats);
            VkExtent2D extent = ChooseSwapExtent(capabilities);

            uint32_t image_count = capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
            {
                image_count = capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            create_info.surface = m_surface;
            create_info.minImageCount = image_count;
            create_info.imageFormat = surface_format.format;
            create_info.imageColorSpace = surface_format.colorSpace;
            create_info.imageExtent = extent;
            create_info.imageArrayLayers = 1;
            create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            uint32_t queue_family_indices[] = { m_graphics_queue_family, m_present_queue_family };
            if (m_graphics_queue_family != m_present_queue_family)
            {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
            }
            else
            {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            create_info.preTransform = capabilities.currentTransform;
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
            create_info.clipped = VK_TRUE;
            create_info.oldSwapchain = VK_NULL_HANDLE;

            CheckVk(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain), "vkCreateSwapchainKHR");

            CheckVk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr), "vkGetSwapchainImagesKHR");
            m_swapchain_images.resize(image_count);
            CheckVk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data()), "vkGetSwapchainImagesKHR");

            m_swapchain_format = surface_format.format;
            m_swapchain_extent = extent;
        }

        void CreateImageViews()
        {
            m_swapchain_image_views.resize(m_swapchain_images.size());
            for (size_t i = 0; i < m_swapchain_images.size(); ++i)
            {
                VkImageViewCreateInfo create_info {};
                create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                create_info.image = m_swapchain_images[i];
                create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                create_info.format = m_swapchain_format;
                create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                create_info.subresourceRange.baseMipLevel = 0;
                create_info.subresourceRange.levelCount = 1;
                create_info.subresourceRange.baseArrayLayer = 0;
                create_info.subresourceRange.layerCount = 1;

                CheckVk(vkCreateImageView(m_device, &create_info, nullptr, &m_swapchain_image_views[i]), "vkCreateImageView");
            }
        }

        uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const
        {
            VkPhysicalDeviceMemoryProperties memory_properties {};
            vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

            for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
            {
                if ((type_filter & (1u << i)) != 0 &&
                    (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            throw std::runtime_error("No compatible Vulkan memory type was found");
        }

        VkFormat FindSupportedFormat(
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features) const
        {
            for (VkFormat format : candidates)
            {
                VkFormatProperties properties {};
                vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &properties);
                if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
                {
                    return format;
                }
                if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
                {
                    return format;
                }
            }

            throw std::runtime_error("No compatible Vulkan image format was found");
        }

        VkFormat FindDepthFormat() const
        {
            return FindSupportedFormat(
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask) const
        {
            VkImageViewCreateInfo create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = image;
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = format;
            create_info.subresourceRange.aspectMask = aspect_mask;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            VkImageView image_view = VK_NULL_HANDLE;
            CheckVk(vkCreateImageView(m_device, &create_info, nullptr, &image_view), "vkCreateImageView");
            return image_view;
        }

        void CreateImage(
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& image_memory) const
        {
            VkImageCreateInfo image_info {};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.extent.width = width;
            image_info.extent.height = height;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.format = format;
            image_info.tiling = tiling;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_info.usage = usage;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            CheckVk(vkCreateImage(m_device, &image_info, nullptr, &image), "vkCreateImage");

            VkMemoryRequirements memory_requirements {};
            vkGetImageMemoryRequirements(m_device, image, &memory_requirements);

            VkMemoryAllocateInfo alloc_info {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = memory_requirements.size;
            alloc_info.memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties);

            CheckVk(vkAllocateMemory(m_device, &alloc_info, nullptr, &image_memory), "vkAllocateMemory");
            CheckVk(vkBindImageMemory(m_device, image, image_memory, 0), "vkBindImageMemory");
        }

        void CreateDepthResources()
        {
            m_depth_format = FindDepthFormat();
            CreateImage(
                m_swapchain_extent.width,
                m_swapchain_extent.height,
                m_depth_format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_depth_image,
                m_depth_image_memory);
            m_depth_image_view = CreateImageView(m_depth_image, m_depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        void CreateRenderAttachment(
            RenderAttachment& attachment,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspect_mask)
        {
            attachment.format = format;
            CreateImage(
                m_swapchain_extent.width,
                m_swapchain_extent.height,
                format,
                VK_IMAGE_TILING_OPTIMAL,
                usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                attachment.image,
                attachment.memory);
            attachment.view = CreateImageView(attachment.image, format, aspect_mask);
        }

        void CreateOffscreenRenderPasses()
        {
            m_depth_format = FindDepthFormat();

            std::array<VkAttachmentDescription, 5> gbuffer_attachments {};
            for (size_t i = 0; i < 4; ++i)
            {
                gbuffer_attachments[i].format = VK_FORMAT_R8G8B8A8_UNORM;
                gbuffer_attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                gbuffer_attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                gbuffer_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                gbuffer_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                gbuffer_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                gbuffer_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                gbuffer_attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            gbuffer_attachments[4].format = m_depth_format;
            gbuffer_attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
            gbuffer_attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            gbuffer_attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            gbuffer_attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            gbuffer_attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            gbuffer_attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            gbuffer_attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            std::array<VkAttachmentReference, 4> color_refs {};
            for (uint32_t i = 0; i < static_cast<uint32_t>(color_refs.size()); ++i)
            {
                color_refs[i].attachment = i;
                color_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            VkAttachmentReference depth_ref {};
            depth_ref.attachment = 4;
            depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription gbuffer_subpass {};
            gbuffer_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            gbuffer_subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
            gbuffer_subpass.pColorAttachments = color_refs.data();
            gbuffer_subpass.pDepthStencilAttachment = &depth_ref;

            VkSubpassDependency gbuffer_dependency {};
            gbuffer_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            gbuffer_dependency.dstSubpass = 0;
            gbuffer_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            gbuffer_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            gbuffer_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo gbuffer_info {};
            gbuffer_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            gbuffer_info.attachmentCount = static_cast<uint32_t>(gbuffer_attachments.size());
            gbuffer_info.pAttachments = gbuffer_attachments.data();
            gbuffer_info.subpassCount = 1;
            gbuffer_info.pSubpasses = &gbuffer_subpass;
            gbuffer_info.dependencyCount = 1;
            gbuffer_info.pDependencies = &gbuffer_dependency;
            CheckVk(vkCreateRenderPass(m_device, &gbuffer_info, nullptr, &m_gbuffer_render_pass), "vkCreateRenderPass(gbuffer)");

            VkAttachmentDescription scene_color {};
            scene_color.format = VK_FORMAT_R8G8B8A8_UNORM;
            scene_color.samples = VK_SAMPLE_COUNT_1_BIT;
            scene_color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            scene_color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            scene_color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            scene_color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            scene_color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            scene_color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentReference scene_color_ref {};
            scene_color_ref.attachment = 0;
            scene_color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription scene_subpass {};
            scene_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            scene_subpass.colorAttachmentCount = 1;
            scene_subpass.pColorAttachments = &scene_color_ref;

            VkSubpassDependency scene_dependency {};
            scene_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            scene_dependency.dstSubpass = 0;
            scene_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            scene_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            scene_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo scene_info {};
            scene_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            scene_info.attachmentCount = 1;
            scene_info.pAttachments = &scene_color;
            scene_info.subpassCount = 1;
            scene_info.pSubpasses = &scene_subpass;
            scene_info.dependencyCount = 1;
            scene_info.pDependencies = &scene_dependency;
            CheckVk(vkCreateRenderPass(m_device, &scene_info, nullptr, &m_scene_result_render_pass), "vkCreateRenderPass(scene_result)");
        }

        void CreateOffscreenResources()
        {
            constexpr VkImageUsageFlags gbuffer_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            CreateRenderAttachment(m_gbuffer_a, VK_FORMAT_R8G8B8A8_UNORM, gbuffer_usage, VK_IMAGE_ASPECT_COLOR_BIT);
            CreateRenderAttachment(m_gbuffer_b, VK_FORMAT_R8G8B8A8_UNORM, gbuffer_usage, VK_IMAGE_ASPECT_COLOR_BIT);
            CreateRenderAttachment(m_gbuffer_c, VK_FORMAT_R8G8B8A8_UNORM, gbuffer_usage, VK_IMAGE_ASPECT_COLOR_BIT);
            CreateRenderAttachment(m_gbuffer_d, VK_FORMAT_R8G8B8A8_UNORM, gbuffer_usage, VK_IMAGE_ASPECT_COLOR_BIT);
            CreateRenderAttachment(
                m_gbuffer_depth,
                m_depth_format,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);
            CreateRenderAttachment(
                m_scene_result,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

            VkImageView gbuffer_attachments[] = {
                m_gbuffer_a.view,
                m_gbuffer_b.view,
                m_gbuffer_c.view,
                m_gbuffer_d.view,
                m_gbuffer_depth.view
            };

            VkFramebufferCreateInfo gbuffer_info {};
            gbuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            gbuffer_info.renderPass = m_gbuffer_render_pass;
            gbuffer_info.attachmentCount = static_cast<uint32_t>(std::size(gbuffer_attachments));
            gbuffer_info.pAttachments = gbuffer_attachments;
            gbuffer_info.width = m_swapchain_extent.width;
            gbuffer_info.height = m_swapchain_extent.height;
            gbuffer_info.layers = 1;
            CheckVk(vkCreateFramebuffer(m_device, &gbuffer_info, nullptr, &m_gbuffer_framebuffer), "vkCreateFramebuffer(gbuffer)");

            VkFramebufferCreateInfo scene_info {};
            scene_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            scene_info.renderPass = m_scene_result_render_pass;
            scene_info.attachmentCount = 1;
            scene_info.pAttachments = &m_scene_result.view;
            scene_info.width = m_swapchain_extent.width;
            scene_info.height = m_swapchain_extent.height;
            scene_info.layers = 1;
            CheckVk(vkCreateFramebuffer(m_device, &scene_info, nullptr, &m_scene_result_framebuffer), "vkCreateFramebuffer(scene_result)");
        }

        void CreateRenderPass()
        {
            m_depth_format = FindDepthFormat();

            VkAttachmentDescription color_attachment {};
            color_attachment.format = m_swapchain_format;
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference color_attachment_ref {};
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depth_attachment {};
            depth_attachment.format = m_depth_format;
            depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depth_attachment_ref {};
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment_ref;
            subpass.pDepthStencilAttachment = &depth_attachment_ref;

            VkSubpassDependency dependency {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

            VkRenderPassCreateInfo render_pass_info {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            render_pass_info.pAttachments = attachments.data();
            render_pass_info.subpassCount = 1;
            render_pass_info.pSubpasses = &subpass;
            render_pass_info.dependencyCount = 1;
            render_pass_info.pDependencies = &dependency;

            CheckVk(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass), "vkCreateRenderPass");
        }

        void CreateFramebuffers()
        {
            m_framebuffers.resize(m_swapchain_image_views.size());
            for (size_t i = 0; i < m_swapchain_image_views.size(); ++i)
            {
                VkImageView attachments[] = { m_swapchain_image_views[i], m_depth_image_view };

                VkFramebufferCreateInfo framebuffer_info {};
                framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_info.renderPass = m_render_pass;
                framebuffer_info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
                framebuffer_info.pAttachments = attachments;
                framebuffer_info.width = m_swapchain_extent.width;
                framebuffer_info.height = m_swapchain_extent.height;
                framebuffer_info.layers = 1;

                CheckVk(vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_framebuffers[i]), "vkCreateFramebuffer");
            }
        }

        void CreateCommandPool()
        {
            VkCommandPoolCreateInfo pool_info {};
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = m_graphics_queue_family;

            CheckVk(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool), "vkCreateCommandPool");
        }

        void CreateCommandBuffers()
        {
            VkCommandBufferAllocateInfo alloc_info {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = m_command_pool;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

            CheckVk(vkAllocateCommandBuffers(m_device, &alloc_info, m_command_buffers.data()), "vkAllocateCommandBuffers");
        }

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory) const
        {
            VkBufferCreateInfo buffer_info {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = size;
            buffer_info.usage = usage;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            CheckVk(vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer), "vkCreateBuffer");

            VkMemoryRequirements memory_requirements {};
            vkGetBufferMemoryRequirements(m_device, buffer, &memory_requirements);

            VkMemoryAllocateInfo alloc_info {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = memory_requirements.size;
            alloc_info.memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties);

            CheckVk(vkAllocateMemory(m_device, &alloc_info, nullptr, &buffer_memory), "vkAllocateMemory");
            CheckVk(vkBindBufferMemory(m_device, buffer, buffer_memory, 0), "vkBindBufferMemory");
        }

        void UploadMemory(VkDeviceMemory memory, const void* data, VkDeviceSize size) const
        {
            void* mapped_data = nullptr;
            CheckVk(vkMapMemory(m_device, memory, 0, size, 0, &mapped_data), "vkMapMemory");
            std::memcpy(mapped_data, data, static_cast<size_t>(size));
            vkUnmapMemory(m_device, memory);
        }

        VkCommandBuffer BeginSingleTimeCommands() const
        {
            VkCommandBufferAllocateInfo alloc_info {};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandPool = m_command_pool;
            alloc_info.commandBufferCount = 1;

            VkCommandBuffer command_buffer = VK_NULL_HANDLE;
            CheckVk(vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer), "vkAllocateCommandBuffers");

            VkCommandBufferBeginInfo begin_info {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            CheckVk(vkBeginCommandBuffer(command_buffer, &begin_info), "vkBeginCommandBuffer");
            return command_buffer;
        }

        void EndSingleTimeCommands(VkCommandBuffer command_buffer) const
        {
            CheckVk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer");

            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            CheckVk(vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE), "vkQueueSubmit");
            CheckVk(vkQueueWaitIdle(m_graphics_queue), "vkQueueWaitIdle");
            vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
        }

        void TransitionImageLayout(
            VkImage image,
            VkImageLayout old_layout,
            VkImageLayout new_layout,
            VkImageAspectFlags aspect_mask) const
        {
            VkCommandBuffer command_buffer = BeginSingleTimeCommands();

            VkImageMemoryBarrier barrier {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = old_layout;
            barrier.newLayout = new_layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspect_mask;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else
            {
                throw std::runtime_error("Unsupported image layout transition");
            }

            vkCmdPipelineBarrier(
                command_buffer,
                source_stage,
                destination_stage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);

            EndSingleTimeCommands(command_buffer);
        }

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const
        {
            VkCommandBuffer command_buffer = BeginSingleTimeCommands();

            VkBufferImageCopy region {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = { width, height, 1 };

            vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            EndSingleTimeCommands(command_buffer);
        }

        void CreateFallbackTexture(RenderAttachment& texture, const std::array<uint8_t, 4>& rgba)
        {
            VkBuffer staging_buffer = VK_NULL_HANDLE;
            VkDeviceMemory staging_memory = VK_NULL_HANDLE;
            CreateBuffer(
                rgba.size(),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging_buffer,
                staging_memory);
            UploadMemory(staging_memory, rgba.data(), rgba.size());

            texture.format = VK_FORMAT_R8G8B8A8_UNORM;
            CreateImage(
                1,
                1,
                texture.format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                texture.image,
                texture.memory);

            TransitionImageLayout(texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
            CopyBufferToImage(staging_buffer, texture.image, 1, 1);
            TransitionImageLayout(texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
            texture.view = CreateImageView(texture.image, texture.format, VK_IMAGE_ASPECT_COLOR_BIT);

            vkDestroyBuffer(m_device, staging_buffer, nullptr);
            vkFreeMemory(m_device, staging_memory, nullptr);
        }

        void CreateSceneSamplerAndFallbackTextures()
        {
            VkSamplerCreateInfo sampler_info {};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.magFilter = VK_FILTER_LINEAR;
            sampler_info.minFilter = VK_FILTER_LINEAR;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.maxAnisotropy = 1.0f;
            sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
            sampler_info.unnormalizedCoordinates = VK_FALSE;
            sampler_info.compareEnable = VK_FALSE;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            CheckVk(vkCreateSampler(m_device, &sampler_info, nullptr, &m_linear_sampler), "vkCreateSampler");
            CreateFallbackTexture(m_fallback_albedo, { 220, 220, 220, 255 });
            CreateFallbackTexture(m_fallback_normal, { 128, 128, 255, 255 });
            std::cerr << "Fallback textures created for missing macOS material inputs." << std::endl;
        }

        VkShaderModule CreateShaderModule(const std::vector<char>& code) const
        {
            if ((code.size() % sizeof(uint32_t)) != 0)
            {
                throw std::runtime_error("SPIR-V shader bytecode has an invalid size");
            }

            VkShaderModuleCreateInfo create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = code.size();
            create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shader_module = VK_NULL_HANDLE;
            CheckVk(vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module), "vkCreateShaderModule");
            return shader_module;
        }

        void CreateSceneDescriptorSetLayout()
        {
            std::array<VkDescriptorSetLayoutBinding, 14> bindings {};
            bindings[0].binding = 0;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

            bindings[1].binding = 1;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            bindings[2].binding = 2;
            bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[2].descriptorCount = 1;
            bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            bindings[3].binding = 13;
            bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[3].descriptorCount = 1;
            bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            for (uint32_t i = 0; i < 5; ++i)
            {
                bindings[4 + i].binding = 16 + i;
                bindings[4 + i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                bindings[4 + i].descriptorCount = 1;
                bindings[4 + i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                bindings[9 + i].binding = 32 + i;
                bindings[9 + i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                bindings[9 + i].descriptorCount = 1;
                bindings[9 + i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            VkDescriptorSetLayoutCreateInfo layout_info {};
            layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
            layout_info.pBindings = bindings.data();

            CheckVk(vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &m_scene_descriptor_set_layout), "vkCreateDescriptorSetLayout");
        }

        void CreateScenePipelineLayout()
        {
            VkPipelineLayoutCreateInfo pipeline_layout_info {};
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount = 1;
            pipeline_layout_info.pSetLayouts = &m_scene_descriptor_set_layout;

            CheckVk(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_scene_pipeline_layout), "vkCreatePipelineLayout");
        }

        void CreateDebugDrawPipeline(VkRenderPass render_pass, bool enable_depth, VkPipeline& pipeline)
        {
            const std::string shader_dir = DOLAS_EDITOR_RUNTIME_SHADER_DIR;
            VkShaderModule vertex_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/debug_draw_vs.spv"));
            VkShaderModule fragment_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/debug_draw_ps.spv"));

            VkPipelineShaderStageCreateInfo vertex_stage_info {};
            vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertex_stage_info.module = vertex_shader_module;
            vertex_stage_info.pName = "VS";

            VkPipelineShaderStageCreateInfo fragment_stage_info {};
            fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragment_stage_info.module = fragment_shader_module;
            fragment_stage_info.pName = "PS";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage_info, fragment_stage_info };

            VkVertexInputBindingDescription binding_description {};
            binding_description.binding = 0;
            binding_description.stride = sizeof(SceneVertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            VkVertexInputAttributeDescription attribute_description {};
            attribute_description.binding = 0;
            attribute_description.location = 0;
            attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_description.offset = offsetof(SceneVertex, position);

            VkPipelineVertexInputStateCreateInfo vertex_input_info {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexBindingDescriptions = &binding_description;
            vertex_input_info.vertexAttributeDescriptionCount = 1;
            vertex_input_info.pVertexAttributeDescriptions = &attribute_description;

            VkPipelineInputAssemblyStateCreateInfo input_assembly {};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineViewportStateCreateInfo viewport_state {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depth_stencil {};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = enable_depth ? VK_TRUE : VK_FALSE;
            depth_stencil.depthWriteEnable = enable_depth ? VK_TRUE : VK_FALSE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

            VkPipelineColorBlendAttachmentState color_blend_attachment {};
            color_blend_attachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            VkPipelineColorBlendStateCreateInfo color_blending {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;

            VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state {};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states));
            dynamic_state.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipeline_info {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = static_cast<uint32_t>(std::size(shader_stages));
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pDepthStencilState = enable_depth ? &depth_stencil : nullptr;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = m_scene_pipeline_layout;
            pipeline_info.renderPass = render_pass;
            pipeline_info.subpass = 0;

            CheckVk(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline), "vkCreateGraphicsPipelines");

            vkDestroyShaderModule(m_device, fragment_shader_module, nullptr);
            vkDestroyShaderModule(m_device, vertex_shader_module, nullptr);
        }

        void CreateGBufferPipeline()
        {
            const std::string shader_dir = DOLAS_EDITOR_RUNTIME_SHADER_DIR;
            VkShaderModule vertex_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/opaque_vs.spv"));
            VkShaderModule fragment_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/opaque_ps.spv"));

            VkPipelineShaderStageCreateInfo vertex_stage_info {};
            vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertex_stage_info.module = vertex_shader_module;
            vertex_stage_info.pName = "VS";

            VkPipelineShaderStageCreateInfo fragment_stage_info {};
            fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragment_stage_info.module = fragment_shader_module;
            fragment_stage_info.pName = "PS";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage_info, fragment_stage_info };

            VkVertexInputBindingDescription binding_description {};
            binding_description.binding = 0;
            binding_description.stride = sizeof(SceneVertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions {};
            attribute_descriptions[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, position) };
            attribute_descriptions[1] = { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, texcoord) };
            attribute_descriptions[2] = { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, normal) };
            attribute_descriptions[3] = { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, tangent) };

            VkPipelineVertexInputStateCreateInfo vertex_input_info {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexBindingDescriptions = &binding_description;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
            vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly {};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineViewportStateCreateInfo viewport_state {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depth_stencil {};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = VK_TRUE;
            depth_stencil.depthWriteEnable = VK_TRUE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

            std::array<VkPipelineColorBlendAttachmentState, 4> blend_attachments {};
            for (VkPipelineColorBlendAttachmentState& attachment : blend_attachments)
            {
                attachment.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }

            VkPipelineColorBlendStateCreateInfo color_blending {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.attachmentCount = static_cast<uint32_t>(blend_attachments.size());
            color_blending.pAttachments = blend_attachments.data();

            VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state {};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states));
            dynamic_state.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipeline_info {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = static_cast<uint32_t>(std::size(shader_stages));
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pDepthStencilState = &depth_stencil;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = m_scene_pipeline_layout;
            pipeline_info.renderPass = m_gbuffer_render_pass;
            pipeline_info.subpass = 0;

            CheckVk(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_gbuffer_pipeline), "vkCreateGraphicsPipelines(gbuffer)");

            vkDestroyShaderModule(m_device, fragment_shader_module, nullptr);
            vkDestroyShaderModule(m_device, vertex_shader_module, nullptr);
        }

        void CreateDeferredPipeline()
        {
            const std::string shader_dir = DOLAS_EDITOR_RUNTIME_SHADER_DIR;
            VkShaderModule vertex_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/deferred_shading_vs.spv"));
            VkShaderModule fragment_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/deferred_shading_ps.spv"));

            VkPipelineShaderStageCreateInfo vertex_stage_info {};
            vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertex_stage_info.module = vertex_shader_module;
            vertex_stage_info.pName = "VS";

            VkPipelineShaderStageCreateInfo fragment_stage_info {};
            fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragment_stage_info.module = fragment_shader_module;
            fragment_stage_info.pName = "PS";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage_info, fragment_stage_info };

            VkVertexInputBindingDescription binding_description {};
            binding_description.binding = 0;
            binding_description.stride = sizeof(SceneVertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions {};
            attribute_descriptions[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, position) };
            attribute_descriptions[1] = { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, texcoord) };

            VkPipelineVertexInputStateCreateInfo vertex_input_info {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexBindingDescriptions = &binding_description;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
            vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly {};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineViewportStateCreateInfo viewport_state {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment {};
            color_blend_attachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            VkPipelineColorBlendStateCreateInfo color_blending {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;

            VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state {};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states));
            dynamic_state.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipeline_info {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = static_cast<uint32_t>(std::size(shader_stages));
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = m_scene_pipeline_layout;
            pipeline_info.renderPass = m_scene_result_render_pass;
            pipeline_info.subpass = 0;

            CheckVk(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_deferred_pipeline), "vkCreateGraphicsPipelines(deferred)");

            vkDestroyShaderModule(m_device, fragment_shader_module, nullptr);
            vkDestroyShaderModule(m_device, vertex_shader_module, nullptr);
        }

        void CreatePresentPipeline()
        {
            const std::string shader_dir = DOLAS_EDITOR_RUNTIME_SHADER_DIR;
            VkShaderModule vertex_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/deferred_shading_vs.spv"));
            VkShaderModule fragment_shader_module = CreateShaderModule(ReadBinaryFile(shader_dir + "/present_scene_ps.spv"));

            VkPipelineShaderStageCreateInfo vertex_stage_info {};
            vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertex_stage_info.module = vertex_shader_module;
            vertex_stage_info.pName = "VS";

            VkPipelineShaderStageCreateInfo fragment_stage_info {};
            fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragment_stage_info.module = fragment_shader_module;
            fragment_stage_info.pName = "PS";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage_info, fragment_stage_info };

            VkVertexInputBindingDescription binding_description {};
            binding_description.binding = 0;
            binding_description.stride = sizeof(SceneVertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions {};
            attribute_descriptions[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, position) };
            attribute_descriptions[1] = { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, texcoord) };

            VkPipelineVertexInputStateCreateInfo vertex_input_info {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexBindingDescriptions = &binding_description;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
            vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly {};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineViewportStateCreateInfo viewport_state {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment {};
            color_blend_attachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            VkPipelineColorBlendStateCreateInfo color_blending {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;

            VkPipelineDepthStencilStateCreateInfo depth_stencil {};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = VK_FALSE;
            depth_stencil.depthWriteEnable = VK_FALSE;

            VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state {};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states));
            dynamic_state.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipeline_info {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = static_cast<uint32_t>(std::size(shader_stages));
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pDepthStencilState = &depth_stencil;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = m_scene_pipeline_layout;
            pipeline_info.renderPass = m_render_pass;
            pipeline_info.subpass = 0;

            CheckVk(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_present_pipeline), "vkCreateGraphicsPipelines(present)");

            vkDestroyShaderModule(m_device, fragment_shader_module, nullptr);
            vkDestroyShaderModule(m_device, vertex_shader_module, nullptr);
        }

        void CreateScenePipelines()
        {
            CreateScenePipelineLayout();
            CreateGBufferPipeline();
            CreateDeferredPipeline();
            CreatePresentPipeline();
            CreateDebugDrawPipeline(m_render_pass, true, m_scene_pipeline);
            CreateDebugDrawPipeline(m_scene_result_render_pass, false, m_scene_result_pipeline);
        }

        void CreateSceneBuffers()
        {
            std::vector<SceneVertex> vertices;
            m_scene_objects.clear();

            auto add_object = [&](std::string name, std::array<float, 4> color, const auto& append_vertices) {
                SceneObject object;
                object.name = std::move(name);
                object.color = color;
                object.first_vertex = static_cast<uint32_t>(vertices.size());
                append_vertices(vertices);
                object.vertex_count = static_cast<uint32_t>(vertices.size()) - object.first_vertex;
                m_scene_objects.push_back(std::move(object));
            };

            add_object("ground_plane", { 0.20f, 0.52f, 0.36f, 1.0f }, [](std::vector<SceneVertex>& out_vertices) {
                AppendQuad(out_vertices, -0.88f, -0.58f, 0.88f, -0.44f, 0.70f);
            });
            add_object("hammer_handle", { 0.64f, 0.36f, 0.18f, 1.0f }, [](std::vector<SceneVertex>& out_vertices) {
                AppendQuad(out_vertices, -0.07f, -0.42f, 0.07f, 0.24f, 0.35f);
            });
            add_object("hammer_head", { 0.68f, 0.70f, 0.68f, 1.0f }, [](std::vector<SceneVertex>& out_vertices) {
                AppendQuad(out_vertices, -0.34f, 0.18f, 0.34f, 0.34f, 0.28f);
            });
            add_object("sphere_0", { 0.35f, 0.58f, 0.90f, 1.0f }, [](std::vector<SceneVertex>& out_vertices) {
                AppendDisc(out_vertices, 0.48f, -0.08f, 0.16f, 0.20f, 32);
            });

            const VkDeviceSize vertex_buffer_size = static_cast<VkDeviceSize>(sizeof(SceneVertex) * vertices.size());
            CreateBuffer(
                vertex_buffer_size,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_scene_vertex_buffer,
                m_scene_vertex_memory);
            UploadMemory(m_scene_vertex_memory, vertices.data(), vertex_buffer_size);

            const std::array<SceneVertex, 6> fullscreen_vertices = {
                MakeSceneVertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
                MakeSceneVertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f),
                MakeSceneVertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f),
                MakeSceneVertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
                MakeSceneVertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f),
                MakeSceneVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f),
            };
            CreateBuffer(
                sizeof(SceneVertex) * fullscreen_vertices.size(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_fullscreen_vertex_buffer,
                m_fullscreen_vertex_memory);
            UploadMemory(m_fullscreen_vertex_memory, fullscreen_vertices.data(), sizeof(SceneVertex) * fullscreen_vertices.size());

            PerViewUniform per_view {};
            per_view.view = IdentityMatrix();
            per_view.projection = IdentityMatrix();
            per_view.camera_position[2] = -1.0f;
            per_view.camera_position[3] = 1.0f;
            CreateBuffer(
                sizeof(PerViewUniform),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_per_view_buffer,
                m_per_view_memory);
            UploadMemory(m_per_view_memory, &per_view, sizeof(per_view));

            PerFrameUniform per_frame {};
            per_frame.light_direction_intensity[0] = 0.25f;
            per_frame.light_direction_intensity[1] = -0.85f;
            per_frame.light_direction_intensity[2] = -0.45f;
            per_frame.light_direction_intensity[3] = 1.2f;
            per_frame.light_color[0] = 1.0f;
            per_frame.light_color[1] = 0.96f;
            per_frame.light_color[2] = 0.88f;
            per_frame.light_color[3] = 1.0f;
            CreateBuffer(
                sizeof(PerFrameUniform),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_per_frame_buffer,
                m_per_frame_memory);
            UploadMemory(m_per_frame_memory, &per_frame, sizeof(per_frame));

            for (SceneObject& object : m_scene_objects)
            {
                PerObjectUniform per_object {};
                per_object.world = IdentityMatrix();
                CreateBuffer(
                    sizeof(PerObjectUniform),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    object.object_buffer,
                    object.object_memory);
                UploadMemory(object.object_memory, &per_object, sizeof(per_object));

                GlobalUniform global_uniform {};
                std::memcpy(global_uniform.debug_draw_color, object.color.data(), sizeof(global_uniform.debug_draw_color));
                CreateBuffer(
                    sizeof(GlobalUniform),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    object.color_buffer,
                    object.color_memory);
                UploadMemory(object.color_memory, &global_uniform, sizeof(global_uniform));
            }
        }

        void CreateSceneDescriptors()
        {
            if (m_scene_objects.empty())
            {
                throw std::runtime_error("Scene draw list is empty");
            }

            std::array<VkDescriptorPoolSize, 3> pool_sizes {};
            pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_sizes[0].descriptorCount = static_cast<uint32_t>(m_scene_objects.size() * 4 + 8);
            pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            pool_sizes[1].descriptorCount = static_cast<uint32_t>(m_scene_objects.size() * 5 + 10);
            pool_sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
            pool_sizes[2].descriptorCount = static_cast<uint32_t>(m_scene_objects.size() * 5 + 10);

            VkDescriptorPoolCreateInfo pool_info {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.maxSets = static_cast<uint32_t>(m_scene_objects.size() + 2);
            pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            pool_info.pPoolSizes = pool_sizes.data();

            CheckVk(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_scene_descriptor_pool), "vkCreateDescriptorPool");

            std::vector<VkDescriptorSetLayout> layouts(m_scene_objects.size() + 2, m_scene_descriptor_set_layout);
            std::vector<VkDescriptorSet> descriptor_sets(m_scene_objects.size() + 2, VK_NULL_HANDLE);

            VkDescriptorSetAllocateInfo alloc_info {};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = m_scene_descriptor_pool;
            alloc_info.descriptorSetCount = static_cast<uint32_t>(descriptor_sets.size());
            alloc_info.pSetLayouts = layouts.data();

            CheckVk(vkAllocateDescriptorSets(m_device, &alloc_info, descriptor_sets.data()), "vkAllocateDescriptorSets");
            m_deferred_descriptor_set = descriptor_sets[m_scene_objects.size()];
            m_present_descriptor_set = descriptor_sets[m_scene_objects.size() + 1];

            for (size_t i = 0; i < m_scene_objects.size(); ++i)
            {
                SceneObject& object = m_scene_objects[i];
                object.descriptor_set = descriptor_sets[i];

                VkDescriptorBufferInfo per_view_info {};
                per_view_info.buffer = m_per_view_buffer;
                per_view_info.range = sizeof(PerViewUniform);

                VkDescriptorBufferInfo per_frame_info {};
                per_frame_info.buffer = m_per_frame_buffer;
                per_frame_info.range = sizeof(PerFrameUniform);

                VkDescriptorBufferInfo per_object_info {};
                per_object_info.buffer = object.object_buffer;
                per_object_info.range = sizeof(PerObjectUniform);

                VkDescriptorBufferInfo global_info {};
                global_info.buffer = object.color_buffer;
                global_info.range = sizeof(GlobalUniform);

                VkDescriptorImageInfo fallback_albedo_info {};
                fallback_albedo_info.imageView = m_fallback_albedo.view;
                fallback_albedo_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkDescriptorImageInfo fallback_normal_info {};
                fallback_normal_info.imageView = m_fallback_normal.view;
                fallback_normal_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkDescriptorImageInfo fallback_sampler_info {};
                fallback_sampler_info.sampler = m_linear_sampler;

                std::array<VkDescriptorImageInfo, 5> sampled_image_infos {};
                sampled_image_infos[0] = fallback_albedo_info;
                sampled_image_infos[1] = fallback_normal_info;
                sampled_image_infos[2] = fallback_albedo_info;
                sampled_image_infos[3] = fallback_albedo_info;
                sampled_image_infos[4] = fallback_albedo_info;

                std::array<VkDescriptorImageInfo, 5> sampler_infos {};
                sampler_infos.fill(fallback_sampler_info);

                std::array<VkWriteDescriptorSet, 14> writes {};
                writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[0].dstSet = object.descriptor_set;
                writes[0].dstBinding = 0;
                writes[0].descriptorCount = 1;
                writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[0].pBufferInfo = &per_view_info;

                writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[1].dstSet = object.descriptor_set;
                writes[1].dstBinding = 1;
                writes[1].descriptorCount = 1;
                writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[1].pBufferInfo = &per_frame_info;

                writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[2].dstSet = object.descriptor_set;
                writes[2].dstBinding = 2;
                writes[2].descriptorCount = 1;
                writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[2].pBufferInfo = &per_object_info;

                writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[3].dstSet = object.descriptor_set;
                writes[3].dstBinding = 13;
                writes[3].descriptorCount = 1;
                writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[3].pBufferInfo = &global_info;

                for (uint32_t binding = 0; binding < 5; ++binding)
                {
                    writes[4 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writes[4 + binding].dstSet = object.descriptor_set;
                    writes[4 + binding].dstBinding = 16 + binding;
                    writes[4 + binding].descriptorCount = 1;
                    writes[4 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    writes[4 + binding].pImageInfo = &sampled_image_infos[binding];

                    writes[9 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writes[9 + binding].dstSet = object.descriptor_set;
                    writes[9 + binding].dstBinding = 32 + binding;
                    writes[9 + binding].descriptorCount = 1;
                    writes[9 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                    writes[9 + binding].pImageInfo = &sampler_infos[binding];
                }

                vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
            }

            UpdateDeferredDescriptorSet();
            UpdatePresentDescriptorSet();
        }

        void UpdateDeferredDescriptorSet()
        {
            if (m_deferred_descriptor_set == VK_NULL_HANDLE)
            {
                return;
            }

            VkDescriptorBufferInfo per_view_info {};
            per_view_info.buffer = m_per_view_buffer;
            per_view_info.range = sizeof(PerViewUniform);

            VkDescriptorBufferInfo per_frame_info {};
            per_frame_info.buffer = m_per_frame_buffer;
            per_frame_info.range = sizeof(PerFrameUniform);

            VkDescriptorBufferInfo dummy_object_info {};
            dummy_object_info.buffer = m_scene_objects.empty() ? m_per_view_buffer : m_scene_objects.front().object_buffer;
            dummy_object_info.range = m_scene_objects.empty() ? sizeof(PerViewUniform) : sizeof(PerObjectUniform);

            VkDescriptorBufferInfo dummy_global_info {};
            dummy_global_info.buffer = m_scene_objects.empty() ? m_per_frame_buffer : m_scene_objects.front().color_buffer;
            dummy_global_info.range = m_scene_objects.empty() ? sizeof(PerFrameUniform) : sizeof(GlobalUniform);

            std::array<VkDescriptorImageInfo, 5> sampled_image_infos {};
            sampled_image_infos[0].imageView = m_gbuffer_a.view;
            sampled_image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            sampled_image_infos[1].imageView = m_gbuffer_b.view;
            sampled_image_infos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            sampled_image_infos[2].imageView = m_gbuffer_c.view;
            sampled_image_infos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            sampled_image_infos[3].imageView = m_gbuffer_d.view;
            sampled_image_infos[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            sampled_image_infos[4].imageView = m_gbuffer_depth.view;
            sampled_image_infos[4].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            std::array<VkDescriptorImageInfo, 5> sampler_infos {};
            for (VkDescriptorImageInfo& sampler_info : sampler_infos)
            {
                sampler_info.sampler = m_linear_sampler;
            }

            std::array<VkWriteDescriptorSet, 14> writes {};
            writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet = m_deferred_descriptor_set;
            writes[0].dstBinding = 0;
            writes[0].descriptorCount = 1;
            writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[0].pBufferInfo = &per_view_info;

            writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[1].dstSet = m_deferred_descriptor_set;
            writes[1].dstBinding = 1;
            writes[1].descriptorCount = 1;
            writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[1].pBufferInfo = &per_frame_info;

            writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[2].dstSet = m_deferred_descriptor_set;
            writes[2].dstBinding = 2;
            writes[2].descriptorCount = 1;
            writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[2].pBufferInfo = &dummy_object_info;

            writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[3].dstSet = m_deferred_descriptor_set;
            writes[3].dstBinding = 13;
            writes[3].descriptorCount = 1;
            writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[3].pBufferInfo = &dummy_global_info;

            for (uint32_t binding = 0; binding < 5; ++binding)
            {
                writes[4 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[4 + binding].dstSet = m_deferred_descriptor_set;
                writes[4 + binding].dstBinding = 16 + binding;
                writes[4 + binding].descriptorCount = 1;
                writes[4 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writes[4 + binding].pImageInfo = &sampled_image_infos[binding];

                writes[9 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[9 + binding].dstSet = m_deferred_descriptor_set;
                writes[9 + binding].dstBinding = 32 + binding;
                writes[9 + binding].descriptorCount = 1;
                writes[9 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                writes[9 + binding].pImageInfo = &sampler_infos[binding];
            }

            vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }

        void UpdatePresentDescriptorSet()
        {
            if (m_present_descriptor_set == VK_NULL_HANDLE)
            {
                return;
            }

            VkDescriptorBufferInfo per_view_info {};
            per_view_info.buffer = m_per_view_buffer;
            per_view_info.range = sizeof(PerViewUniform);

            VkDescriptorBufferInfo per_frame_info {};
            per_frame_info.buffer = m_per_frame_buffer;
            per_frame_info.range = sizeof(PerFrameUniform);

            VkDescriptorBufferInfo dummy_object_info {};
            dummy_object_info.buffer = m_scene_objects.empty() ? m_per_view_buffer : m_scene_objects.front().object_buffer;
            dummy_object_info.range = m_scene_objects.empty() ? sizeof(PerViewUniform) : sizeof(PerObjectUniform);

            VkDescriptorBufferInfo dummy_global_info {};
            dummy_global_info.buffer = m_scene_objects.empty() ? m_per_frame_buffer : m_scene_objects.front().color_buffer;
            dummy_global_info.range = m_scene_objects.empty() ? sizeof(PerFrameUniform) : sizeof(GlobalUniform);

            VkDescriptorImageInfo scene_result_info {};
            scene_result_info.imageView = m_scene_result.view;
            scene_result_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo fallback_info {};
            fallback_info.imageView = m_fallback_albedo.view;
            fallback_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            std::array<VkDescriptorImageInfo, 5> sampled_image_infos {};
            sampled_image_infos[0] = scene_result_info;
            sampled_image_infos[1] = fallback_info;
            sampled_image_infos[2] = fallback_info;
            sampled_image_infos[3] = fallback_info;
            sampled_image_infos[4] = fallback_info;

            std::array<VkDescriptorImageInfo, 5> sampler_infos {};
            for (VkDescriptorImageInfo& sampler_info : sampler_infos)
            {
                sampler_info.sampler = m_linear_sampler;
            }

            std::array<VkWriteDescriptorSet, 14> writes {};
            writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet = m_present_descriptor_set;
            writes[0].dstBinding = 0;
            writes[0].descriptorCount = 1;
            writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[0].pBufferInfo = &per_view_info;

            writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[1].dstSet = m_present_descriptor_set;
            writes[1].dstBinding = 1;
            writes[1].descriptorCount = 1;
            writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[1].pBufferInfo = &per_frame_info;

            writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[2].dstSet = m_present_descriptor_set;
            writes[2].dstBinding = 2;
            writes[2].descriptorCount = 1;
            writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[2].pBufferInfo = &dummy_object_info;

            writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[3].dstSet = m_present_descriptor_set;
            writes[3].dstBinding = 13;
            writes[3].descriptorCount = 1;
            writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[3].pBufferInfo = &dummy_global_info;

            for (uint32_t binding = 0; binding < 5; ++binding)
            {
                writes[4 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[4 + binding].dstSet = m_present_descriptor_set;
                writes[4 + binding].dstBinding = 16 + binding;
                writes[4 + binding].descriptorCount = 1;
                writes[4 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writes[4 + binding].pImageInfo = &sampled_image_infos[binding];

                writes[9 + binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[9 + binding].dstSet = m_present_descriptor_set;
                writes[9 + binding].dstBinding = 32 + binding;
                writes[9 + binding].descriptorCount = 1;
                writes[9 + binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                writes[9 + binding].pImageInfo = &sampler_infos[binding];
            }

            vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }

        void CreateSceneResources()
        {
            CreateSceneDescriptorSetLayout();
            CreateScenePipelines();
            CreateSceneBuffers();
            CreateSceneDescriptors();
            std::cerr << "Scene draw resources created: objects=" << m_scene_objects.size() << std::endl;
        }

        void CreateSyncObjects()
        {
            VkSemaphoreCreateInfo semaphore_info {};
            semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fence_info {};
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (int i = 0; i < kMaxFramesInFlight; ++i)
            {
                CheckVk(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]), "vkCreateSemaphore");
                CheckVk(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]), "vkCreateSemaphore");
                CheckVk(vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]), "vkCreateFence");
            }
        }

        void CreateImGuiDescriptorPool()
        {
            VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

            VkDescriptorPoolCreateInfo pool_info {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * static_cast<uint32_t>(std::size(pool_sizes));
            pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
            pool_info.pPoolSizes = pool_sizes;

            CheckVk(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_imgui_descriptor_pool), "vkCreateDescriptorPool");
        }

        void RemoveSceneResultImGuiTexture()
        {
            if (m_imgui_scene_result_descriptor != VK_NULL_HANDLE && m_imgui_backend_initialized)
            {
                ImGui_ImplVulkan_RemoveTexture(m_imgui_scene_result_descriptor);
            }
            m_imgui_scene_result_descriptor = VK_NULL_HANDLE;
        }

        void RegisterSceneResultImGuiTexture()
        {
            if (!m_imgui_backend_initialized || m_scene_result.view == VK_NULL_HANDLE || m_linear_sampler == VK_NULL_HANDLE)
            {
                return;
            }

            RemoveSceneResultImGuiTexture();
            m_imgui_scene_result_descriptor =
                ImGui_ImplVulkan_AddTexture(m_linear_sampler, m_scene_result.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        void ConfigureDockLayout(ImGuiID dockspace_id, const ImVec2& dockspace_size)
        {
            if (m_dock_layout_initialized || dockspace_size.x <= 0.0f || dockspace_size.y <= 0.0f)
            {
                return;
            }

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

            ImGuiID main_node = dockspace_id;
            ImGuiID left_node = 0;
            ImGuiID right_node = 0;
            ImGuiID bottom_node = 0;
            ImGui::DockBuilderSplitNode(main_node, ImGuiDir_Left, 0.20f, &left_node, &main_node);
            ImGui::DockBuilderSplitNode(main_node, ImGuiDir_Right, 0.22f, &right_node, &main_node);
            ImGui::DockBuilderSplitNode(main_node, ImGuiDir_Down, 0.24f, &bottom_node, &main_node);

            ImGui::DockBuilderDockWindow("SCENE HIERARCHY", left_node);
            ImGui::DockBuilderDockWindow("VIEWPORT", main_node);
            ImGui::DockBuilderDockWindow("PROPERTIES", right_node);
            ImGui::DockBuilderDockWindow("CONTENT BROWSER", bottom_node);
            ImGui::DockBuilderFinish(dockspace_id);
            m_dock_layout_initialized = true;
        }

        void BuildDockSpace()
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::Begin("DolasEditor Main DockSpace", nullptr, window_flags);
            ImGui::PopStyleVar(2);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                    {
                        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Reset Layout"))
                    {
                        m_dock_layout_initialized = false;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            const ImGuiID dockspace_id = ImGui::GetID("DolasEditorDockSpace");
            ConfigureDockLayout(dockspace_id, ImGui::GetContentRegionAvail());
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
            ImGui::End();
        }

        void BuildViewportWindow()
        {
            ImGui::Begin("VIEWPORT");
            const ImVec2 available_size = ImGui::GetContentRegionAvail();
            if (m_imgui_scene_result_descriptor == VK_NULL_HANDLE || available_size.x <= 1.0f || available_size.y <= 1.0f)
            {
                ImGui::TextUnformatted("SceneResult unavailable");
                ImGui::End();
                return;
            }

            const float source_aspect =
                static_cast<float>(std::max(1u, m_swapchain_extent.width)) /
                static_cast<float>(std::max(1u, m_swapchain_extent.height));
            ImVec2 image_size = available_size;
            if ((image_size.x / image_size.y) > source_aspect)
            {
                image_size.x = image_size.y * source_aspect;
            }
            else
            {
                image_size.y = image_size.x / source_aspect;
            }

            const float indent_x = std::max(0.0f, (available_size.x - image_size.x) * 0.5f);
            const float indent_y = std::max(0.0f, (available_size.y - image_size.y) * 0.5f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent_x);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + indent_y);
            const ImTextureID texture_id =
                static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(m_imgui_scene_result_descriptor));
            ImGui::Image(
                texture_id,
                image_size,
                ImVec2(0.0f, 0.0f),
                ImVec2(1.0f, 1.0f));
            ImGui::End();
        }

        void BuildHierarchyWindow()
        {
            ImGui::Begin("SCENE HIERARCHY");
            if (ImGui::Button("Reset Scene"))
            {
                m_selected_object_index = 0;
            }
            ImGui::SameLine();
            ImGui::Button("Add Entity");
            ImGui::Separator();
            ImGui::TextUnformatted("default_scene");
            for (int i = 0; i < static_cast<int>(m_scene_objects.size()); ++i)
            {
                const bool selected = (i == m_selected_object_index);
                if (ImGui::Selectable(m_scene_objects[static_cast<size_t>(i)].name.c_str(), selected))
                {
                    m_selected_object_index = i;
                }
            }
            ImGui::End();
        }

        void BuildPropertiesWindow()
        {
            ImGui::Begin("PROPERTIES");
            if (!m_scene_objects.empty())
            {
                m_selected_object_index =
                    std::clamp(m_selected_object_index, 0, static_cast<int>(m_scene_objects.size()) - 1);
                const SceneObject& object = m_scene_objects[static_cast<size_t>(m_selected_object_index)];
                ImGui::Text("Selected Object: %s", object.name.c_str());
                ImGui::Separator();
                ImGui::TextUnformatted("Transform");
                ImGui::TextUnformatted("Position    0.000    0.000    0.000");
                ImGui::TextUnformatted("Rotation    0.000    0.000    0.000");
                ImGui::TextUnformatted("Scale       1.000    1.000    1.000");
                ImGui::Separator();
                ImGui::TextUnformatted("Material");
                ImGui::Text("Base Color  %.2f  %.2f  %.2f", object.color[0], object.color[1], object.color[2]);
                ImGui::Separator();
            }
            ImGui::TextUnformatted("Renderer");
            ImGui::TextUnformatted("Vulkan / MoltenVK");
            ImGui::Text("Swapchain   %u x %u", m_swapchain_extent.width, m_swapchain_extent.height);
            ImGui::TextUnformatted("SceneResult offscreen color target");
            ImGui::End();
        }

        void BuildContentBrowserWindow()
        {
            ImGui::Begin("CONTENT BROWSER");
            ImGui::TextUnformatted("content/");
            ImGui::Separator();

            constexpr const char* kItems[] = {
                "default_scene.scene",
                "debug_draw.material",
                "deferred_shading.material",
                "sky_box.material",
                "golden_gate_hills_4k.hdr",
                "horn-koppe_spring_4k.hdr",
                "debug_draw_vs.hlsl",
                "debug_draw_ps.hlsl",
                "deferred_shading_ps.hlsl",
                "present_scene_ps.hlsl"
            };

            const float item_width = 120.0f;
            const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / item_width));
            ImGui::Columns(columns, nullptr, false);
            for (const char* item : kItems)
            {
                ImGui::Button(item, ImVec2(item_width - 10.0f, 34.0f));
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::End();
        }

        void BuildEditorUi()
        {
            BuildDockSpace();
            BuildHierarchyWindow();
            BuildViewportWindow();
            BuildPropertiesWindow();
            BuildContentBrowserWindow();
        }

        void RecordScene(VkCommandBuffer command_buffer, VkPipeline pipeline)
        {
            if (pipeline == VK_NULL_HANDLE || m_scene_vertex_buffer == VK_NULL_HANDLE)
            {
                return;
            }

            VkViewport viewport {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_swapchain_extent.width);
            viewport.height = static_cast<float>(m_swapchain_extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.offset = { 0, 0 };
            scissor.extent = m_swapchain_extent;

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdSetViewport(command_buffer, 0, 1, &viewport);
            vkCmdSetScissor(command_buffer, 0, 1, &scissor);

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_scene_vertex_buffer, offsets);

            for (const SceneObject& object : m_scene_objects)
            {
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_scene_pipeline_layout,
                    0,
                    1,
                    &object.descriptor_set,
                    0,
                    nullptr);
                vkCmdDraw(command_buffer, object.vertex_count, 1, object.first_vertex, 0);
            }
        }

        void RecordGBufferPass(VkCommandBuffer command_buffer)
        {
            std::array<VkClearValue, 5> clear_values {};
            clear_values[0].color = { { 0.50f, 0.50f, 1.00f, 1.0f } };
            clear_values[1].color = { { 0.04f, 0.04f, 0.05f, 1.0f } };
            clear_values[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clear_values[3].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clear_values[4].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo render_pass_info {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = m_gbuffer_render_pass;
            render_pass_info.framebuffer = m_gbuffer_framebuffer;
            render_pass_info.renderArea.offset = { 0, 0 };
            render_pass_info.renderArea.extent = m_swapchain_extent;
            render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            RecordScene(command_buffer, m_gbuffer_pipeline);
            vkCmdEndRenderPass(command_buffer);
        }

        void BarrierGBufferForDeferred(VkCommandBuffer command_buffer)
        {
            std::array<VkImageMemoryBarrier, 5> barriers {};
            RenderAttachment* color_attachments[] = { &m_gbuffer_a, &m_gbuffer_b, &m_gbuffer_c, &m_gbuffer_d };
            for (size_t i = 0; i < std::size(color_attachments); ++i)
            {
                barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barriers[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barriers[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barriers[i].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barriers[i].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barriers[i].image = color_attachments[i]->image;
                barriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barriers[i].subresourceRange.levelCount = 1;
                barriers[i].subresourceRange.layerCount = 1;
            }

            barriers[4].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[4].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barriers[4].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barriers[4].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            barriers[4].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            barriers[4].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[4].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[4].image = m_gbuffer_depth.image;
            barriers[4].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barriers[4].subresourceRange.levelCount = 1;
            barriers[4].subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                command_buffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                static_cast<uint32_t>(barriers.size()),
                barriers.data());
        }

        void RecordSceneResultPass(VkCommandBuffer command_buffer)
        {
            VkClearValue clear_value {};
            clear_value.color = { { 0.08f, 0.10f, 0.14f, 1.0f } };

            VkRenderPassBeginInfo render_pass_info {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = m_scene_result_render_pass;
            render_pass_info.framebuffer = m_scene_result_framebuffer;
            render_pass_info.renderArea.offset = { 0, 0 };
            render_pass_info.renderArea.extent = m_swapchain_extent;
            render_pass_info.clearValueCount = 1;
            render_pass_info.pClearValues = &clear_value;

            vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            if (m_deferred_pipeline != VK_NULL_HANDLE && m_fullscreen_vertex_buffer != VK_NULL_HANDLE)
            {
                VkViewport viewport {};
                viewport.width = static_cast<float>(m_swapchain_extent.width);
                viewport.height = static_cast<float>(m_swapchain_extent.height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;

                VkRect2D scissor {};
                scissor.extent = m_swapchain_extent;

                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferred_pipeline);
                vkCmdSetViewport(command_buffer, 0, 1, &viewport);
                vkCmdSetScissor(command_buffer, 0, 1, &scissor);
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_fullscreen_vertex_buffer, offsets);
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_scene_pipeline_layout,
                    0,
                    1,
                    &m_deferred_descriptor_set,
                    0,
                    nullptr);
                vkCmdDraw(command_buffer, 6, 1, 0, 0);
            }
            RecordScene(command_buffer, m_scene_result_pipeline);
            vkCmdEndRenderPass(command_buffer);
        }

        void BarrierSceneResultForPresent(VkCommandBuffer command_buffer)
        {
            VkImageMemoryBarrier barrier {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_scene_result.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                command_buffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);
        }

        void RecordPresentSceneResult(VkCommandBuffer command_buffer)
        {
            if (m_present_pipeline == VK_NULL_HANDLE ||
                m_present_descriptor_set == VK_NULL_HANDLE ||
                m_fullscreen_vertex_buffer == VK_NULL_HANDLE)
            {
                return;
            }

            VkViewport viewport {};
            viewport.width = static_cast<float>(m_swapchain_extent.width);
            viewport.height = static_cast<float>(m_swapchain_extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.extent = m_swapchain_extent;

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_present_pipeline);
            vkCmdSetViewport(command_buffer, 0, 1, &viewport);
            vkCmdSetScissor(command_buffer, 0, 1, &scissor);

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_fullscreen_vertex_buffer, offsets);
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_scene_pipeline_layout,
                0,
                1,
                &m_present_descriptor_set,
                0,
                nullptr);
            vkCmdDraw(command_buffer, 6, 1, 0, 0);
        }

        void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index)
        {
            VkCommandBufferBeginInfo begin_info {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            CheckVk(vkBeginCommandBuffer(command_buffer, &begin_info), "vkBeginCommandBuffer");

            RecordGBufferPass(command_buffer);
            BarrierGBufferForDeferred(command_buffer);
            RecordSceneResultPass(command_buffer);
            BarrierSceneResultForPresent(command_buffer);

            std::array<VkClearValue, 2> clear_values {};
            clear_values[0].color = { { 0.06f, 0.07f, 0.09f, 1.0f } };
            clear_values[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo render_pass_info {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = m_render_pass;
            render_pass_info.framebuffer = m_framebuffers[image_index];
            render_pass_info.renderArea.offset = { 0, 0 };
            render_pass_info.renderArea.extent = m_swapchain_extent;
            render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            RecordPresentSceneResult(command_buffer);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
            vkCmdEndRenderPass(command_buffer);

            CheckVk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer");
        }

        void DrawFrame()
        {
            CheckVk(vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX), "vkWaitForFences");

            uint32_t image_index = 0;
            VkResult acquire_result = vkAcquireNextImageKHR(
                m_device,
                m_swapchain,
                UINT64_MAX,
                m_image_available_semaphores[m_current_frame],
                VK_NULL_HANDLE,
                &image_index);

            if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                RecreateSwapchain();
                return;
            }
            CheckVk(acquire_result, "vkAcquireNextImageKHR");

            CheckVk(vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]), "vkResetFences");
            CheckVk(vkResetCommandBuffer(m_command_buffers[m_current_frame], 0), "vkResetCommandBuffer");

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            BuildEditorUi();
            ImGui::Render();

            RecordCommandBuffer(m_command_buffers[m_current_frame], image_index);

            VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
            VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };

            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = wait_semaphores;
            submit_info.pWaitDstStageMask = wait_stages;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = signal_semaphores;

            CheckVk(vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]), "vkQueueSubmit");

            VkPresentInfoKHR present_info {};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = signal_semaphores;
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = &image_index;

            VkResult present_result = vkQueuePresentKHR(m_present_queue, &present_info);
            if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized)
            {
                m_framebuffer_resized = false;
                RecreateSwapchain();
            }
            else
            {
                CheckVk(present_result, "vkQueuePresentKHR");
            }

            m_current_frame = (m_current_frame + 1) % kMaxFramesInFlight;
        }

        void RecreateSwapchain()
        {
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);
            while (width == 0 || height == 0)
            {
                glfwWaitEvents();
                glfwGetFramebufferSize(m_window, &width, &height);
            }

            CheckVk(vkDeviceWaitIdle(m_device), "vkDeviceWaitIdle");
            CleanupSwapchain();
            CreateSwapchain();
            CreateImageViews();
            CreateDepthResources();
            CreateOffscreenResources();
            UpdateDeferredDescriptorSet();
            UpdatePresentDescriptorSet();
            RegisterSceneResultImGuiTexture();
            CreateFramebuffers();
        }

        void CleanupSwapchain()
        {
            for (VkFramebuffer framebuffer : m_framebuffers)
            {
                vkDestroyFramebuffer(m_device, framebuffer, nullptr);
            }
            m_framebuffers.clear();

            RemoveSceneResultImGuiTexture();
            CleanupOffscreenResources();

            if (m_depth_image_view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(m_device, m_depth_image_view, nullptr);
                m_depth_image_view = VK_NULL_HANDLE;
            }
            if (m_depth_image != VK_NULL_HANDLE)
            {
                vkDestroyImage(m_device, m_depth_image, nullptr);
                m_depth_image = VK_NULL_HANDLE;
            }
            if (m_depth_image_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_device, m_depth_image_memory, nullptr);
                m_depth_image_memory = VK_NULL_HANDLE;
            }

            for (VkImageView image_view : m_swapchain_image_views)
            {
                vkDestroyImageView(m_device, image_view, nullptr);
            }
            m_swapchain_image_views.clear();

            if (m_swapchain != VK_NULL_HANDLE)
            {
                vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
                m_swapchain = VK_NULL_HANDLE;
            }
        }

        void DestroyAttachment(RenderAttachment& attachment) const
        {
            if (attachment.view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(m_device, attachment.view, nullptr);
                attachment.view = VK_NULL_HANDLE;
            }
            if (attachment.image != VK_NULL_HANDLE)
            {
                vkDestroyImage(m_device, attachment.image, nullptr);
                attachment.image = VK_NULL_HANDLE;
            }
            if (attachment.memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_device, attachment.memory, nullptr);
                attachment.memory = VK_NULL_HANDLE;
            }
            attachment.format = VK_FORMAT_UNDEFINED;
        }

        void CleanupOffscreenResources()
        {
            if (m_gbuffer_framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(m_device, m_gbuffer_framebuffer, nullptr);
                m_gbuffer_framebuffer = VK_NULL_HANDLE;
            }
            if (m_scene_result_framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(m_device, m_scene_result_framebuffer, nullptr);
                m_scene_result_framebuffer = VK_NULL_HANDLE;
            }

            DestroyAttachment(m_gbuffer_a);
            DestroyAttachment(m_gbuffer_b);
            DestroyAttachment(m_gbuffer_c);
            DestroyAttachment(m_gbuffer_d);
            DestroyAttachment(m_gbuffer_depth);
            DestroyAttachment(m_scene_result);
        }

        void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory) const
        {
            if (buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(m_device, buffer, nullptr);
                buffer = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
        }

        void CleanupSceneResources()
        {
            if (m_scene_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_device, m_scene_pipeline, nullptr);
                m_scene_pipeline = VK_NULL_HANDLE;
            }
            if (m_scene_result_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_device, m_scene_result_pipeline, nullptr);
                m_scene_result_pipeline = VK_NULL_HANDLE;
            }
            if (m_gbuffer_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_device, m_gbuffer_pipeline, nullptr);
                m_gbuffer_pipeline = VK_NULL_HANDLE;
            }
            if (m_deferred_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_device, m_deferred_pipeline, nullptr);
                m_deferred_pipeline = VK_NULL_HANDLE;
            }
            if (m_present_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_device, m_present_pipeline, nullptr);
                m_present_pipeline = VK_NULL_HANDLE;
            }
            if (m_scene_pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(m_device, m_scene_pipeline_layout, nullptr);
                m_scene_pipeline_layout = VK_NULL_HANDLE;
            }
            if (m_scene_descriptor_pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_device, m_scene_descriptor_pool, nullptr);
                m_scene_descriptor_pool = VK_NULL_HANDLE;
            }
            if (m_scene_descriptor_set_layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(m_device, m_scene_descriptor_set_layout, nullptr);
                m_scene_descriptor_set_layout = VK_NULL_HANDLE;
            }

            DestroyBuffer(m_per_view_buffer, m_per_view_memory);
            DestroyBuffer(m_per_frame_buffer, m_per_frame_memory);
            DestroyBuffer(m_scene_vertex_buffer, m_scene_vertex_memory);
            DestroyBuffer(m_fullscreen_vertex_buffer, m_fullscreen_vertex_memory);
            for (SceneObject& object : m_scene_objects)
            {
                DestroyBuffer(object.object_buffer, object.object_memory);
                DestroyBuffer(object.color_buffer, object.color_memory);
                object.descriptor_set = VK_NULL_HANDLE;
            }
            m_scene_objects.clear();
            m_deferred_descriptor_set = VK_NULL_HANDLE;
            m_present_descriptor_set = VK_NULL_HANDLE;

            if (m_linear_sampler != VK_NULL_HANDLE)
            {
                vkDestroySampler(m_device, m_linear_sampler, nullptr);
                m_linear_sampler = VK_NULL_HANDLE;
            }
            DestroyAttachment(m_fallback_albedo);
            DestroyAttachment(m_fallback_normal);
        }

        void Cleanup()
        {
            if (m_device != VK_NULL_HANDLE)
            {
                vkDeviceWaitIdle(m_device);
            }

            RemoveSceneResultImGuiTexture();
            if (m_imgui_backend_initialized)
            {
                ImGui_ImplVulkan_Shutdown();
                m_imgui_backend_initialized = false;
            }
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            if (m_imgui_descriptor_pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_device, m_imgui_descriptor_pool, nullptr);
            }

            CleanupSceneResources();
            CleanupSwapchain();

            if (m_render_pass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(m_device, m_render_pass, nullptr);
            }
            if (m_gbuffer_render_pass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(m_device, m_gbuffer_render_pass, nullptr);
                m_gbuffer_render_pass = VK_NULL_HANDLE;
            }
            if (m_scene_result_render_pass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(m_device, m_scene_result_render_pass, nullptr);
                m_scene_result_render_pass = VK_NULL_HANDLE;
            }

            for (int i = 0; i < kMaxFramesInFlight; ++i)
            {
                if (m_image_available_semaphores[i] != VK_NULL_HANDLE)
                {
                    vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
                }
                if (m_render_finished_semaphores[i] != VK_NULL_HANDLE)
                {
                    vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
                }
                if (m_in_flight_fences[i] != VK_NULL_HANDLE)
                {
                    vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
                }
            }

            if (m_command_pool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(m_device, m_command_pool, nullptr);
            }
            if (m_device != VK_NULL_HANDLE)
            {
                vkDestroyDevice(m_device, nullptr);
            }
            if (m_surface != VK_NULL_HANDLE)
            {
                vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            }
            if (m_instance != VK_NULL_HANDLE)
            {
                vkDestroyInstance(m_instance, nullptr);
            }
            if (m_window)
            {
                glfwDestroyWindow(m_window);
            }
            glfwTerminate();
        }
    };
}

int main()
{
    try
    {
        MacOSVulkanEditor app;
        app.Run();
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "DolasEditor failed: " << exception.what() << std::endl;
        return 1;
    }
}
