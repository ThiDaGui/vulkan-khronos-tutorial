
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_include_directories(VulkanCppModule PRIVATE
        "${Vulkan_INCLUDE_DIR}"
)

target_link_libraries(VulkanCppModule PUBLIC
        Vulkan::Vulkan
)

set_target_properties(VulkanCppModule PROPERTIES
        CXX_STANDARD 20
)

target_sources(VulkanCppModule PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS "${Vulkan_INCLUDE_DIR}"
        FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)

add_executable(drawTriangleCpp
        helloTriangleApplication.cpp
        vulkanUtils.cpp
        main.cpp

        helloTriangleApplication.hh
        vulkanUtils.hh
)

target_include_directories(drawTriangleCpp PRIVATE
        "${Vulkan_INCLUDE_DIR}"
        "${CMAKE_BINARY_DIR}"
)

set_target_properties(drawTriangleCpp PROPERTIES
        CXX_STANDARD 20
)

target_link_libraries(drawTriangleCpp PRIVATE
        Vulkan::cppm
        glfw
)

add_dependencies(drawTriangleCpp triangle_slang)