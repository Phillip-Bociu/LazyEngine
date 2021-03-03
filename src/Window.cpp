#include "headers/Window.hpp"
#include "headers/Log.hpp"

namespace lzy
{
    GLFWwindow* Window::window = nullptr;

    void Window::Init(const std::string_view title, const glm::uvec2& res)
    {
        if(!glfwInit())
        {
            
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


        window = glfwCreateWindow(res.x,res.y, title.data(), nullptr,nullptr);
    }

    void Window::Shutdown()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

}