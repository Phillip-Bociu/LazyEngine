#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lzy
{


    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer();

        void Init();
    private:
        void createInstance();
        void pickPhysicalDevice();
        void createDevice();
    private:
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
    };

}