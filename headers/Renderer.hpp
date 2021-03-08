#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <limits>
#include <vector>
#include "headers/fvector.hpp"

namespace lzy
{

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        fvector<VkSurfaceFormatKHR> formats;
        fvector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        static constexpr uint32_t invalid = std::numeric_limits<uint32_t>::max();

        uint32_t graphicsFamily = invalid;
        uint32_t presentFamily = invalid;
        uint32_t computeFamily = invalid;

        bool isComplete() const;
    };

    namespace Renderer
    {
        struct Reqs
        {
            VkInstance instance;
            VkPhysicalDevice physicalDevice;
            VkDevice device;
            VkQueue graphicsQueue;
            VkQueue presentQueue;
            VkSurfaceKHR surface;
            VkSwapchainKHR swapchain;
            VkExtent2D swapchainExtent;
            VkFormat swapchainFormat;
            fvector<VkImage> swapchainImages;

///DEBUG VARS
#ifdef DEBUG
            VkDebugUtilsMessengerEXT debugMessenger;
#endif
        };

        void Init(Reqs* reqs);
        void Shutdown(Reqs* reqs);

        void createInstance(VkInstance* instance);
        void setupDebugMessenger(VkDebugUtilsMessengerEXT* debugMessenger, VkInstance instance);
        void pickPhysicalDevice(VkPhysicalDevice *physicalDevice, VkInstance instance, VkSurfaceKHR surface, QueueFamilyIndices *qFam);
        void createDevice(VkDevice *device, VkPhysicalDevice physicalDevice, QueueFamilyIndices *qFam);
        void getQueues(VkQueue *graphicsQueue, VkQueue *presentQueue, VkDevice device, QueueFamilyIndices* qFam);
        void createSurface(VkSurfaceKHR* surface, VkInstance VkInstance);
        void createSwapchain(VkSwapchainKHR *swapchain, VkFormat *swapchainFormat, VkExtent2D *swapchainExtent, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueFamilyIndices *qFam);
        QueueFamilyIndices findQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueFamilyIndices *qFam);
    }

}