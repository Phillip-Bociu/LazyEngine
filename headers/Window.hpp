#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string_view>
#include <glm/glm.hpp>

namespace lzy
{


    class Window
    {
    public:
        static void Init(const std::string_view title, const glm::uvec2& res = {1980, 1080});
        static inline bool shouldClose() { return glfwWindowShouldClose(window);}
        static void Shutdown();
    private:
        static GLFWwindow* window;
        static glm::uvec2 res;
    };

}
