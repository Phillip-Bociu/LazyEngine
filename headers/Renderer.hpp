#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <limits>
#include <vector>

namespace lzy
{

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        static constexpr uint32_t invalid = std::numeric_limits<uint32_t>::max();

        uint32_t graphicsFamily = invalid;
        uint32_t presentFamily = invalid;
        uint32_t computeFamily = invalid;

        bool isComplete() const;
    };

    class Renderer
    {
    public:
        void Init();
        void Shutdown();

    private:

        void createInstance();
        void setupDebugMessenger();
        void pickPhysicalDevice(QueueFamilyIndices *qFam);
        void createDevice(QueueFamilyIndices* qFam);
        void createSurface();
        void createSwapchain(QueueFamilyIndices* qFam);
        QueueFamilyIndices findQueueFamily(VkPhysicalDevice physicalDevice);
        bool isDeviceSuitable(VkPhysicalDevice physicalDevice, QueueFamilyIndices *qFam);

    private:
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        VkExtent2D swapchainExtent;
        VkFormat swapchainFormat;
        std::vector<VkImage> swapchainImages;
        ///DEBUG VARS
        #ifdef DEBUG

        VkDebugUtilsMessengerEXT debugMessenger;

#endif

    };

}