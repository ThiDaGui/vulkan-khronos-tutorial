#include <cstdlib>
#include <exception>
#include <iostream>

#include "engine/vk_engine.hh"

int main(int argc, char *argv[]) {
    try {
        vk_tutorial::VkEngine app{};
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
