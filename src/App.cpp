#include "headers/App.hpp"
#include <stdexcept>
#include <iostream>

namespace lzy
{
    TestApp::TestApp()
    {
        try
        {
            Window::Init("TestApp", {WIDTH, HEIGHT});
            renderer.Init();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    TestApp::~TestApp()
    {
        renderer.Shutdown();
        Window::Shutdown();
    }

    void TestApp::run()
    {
        while (!Window::ShouldClose())
        {
            glfwPollEvents();
        }
    }
}