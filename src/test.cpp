#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "headers/App.hpp"

int main()
{
    lzy::TestApp app{};


    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}