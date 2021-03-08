#pragma once
#include "Window.hpp"
#include "Renderer.hpp"

namespace lzy
{
    class TestApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        TestApp();
        ~TestApp();
        void run();

    private:
        Renderer::Reqs testReqs;
    };
}