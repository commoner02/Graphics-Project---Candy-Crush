cmake_minimum_required(VERSION 3.20)
project(candy_pyramid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# GLM is header-only; find package or rely on system include
find_package(glm QUIET)

add_executable(candy_pyramid
    src/main.cpp
    src/Mesh.cpp
    src/Primitives.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Shape.cpp
    src/Board.cpp
    src/HUD.cpp
)

target_include_directories(candy_pyramid PRIVATE include)

if (TARGET glm::glm)
    target_link_libraries(candy_pyramid PRIVATE glfw GLEW::GLEW OpenGL::GL glm::glm)
else()
    target_link_libraries(candy_pyramid PRIVATE glfw GLEW::GLEW OpenGL::GL)
endif()

# Copy shaders folder next to binary
add_custom_command(TARGET candy_pyramid POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:candy_pyramid>/shaders)
