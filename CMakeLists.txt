cmake_minimum_required(VERSION 3.20)
project(LearnOpenGL)

## Find dependencies
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(freetype REQUIRED)
find_package(imgui REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(OpenMP)

set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)
set (CMAKE_CXX_STANDARD 20)

# enable asan
# add_compile_options(-ggdb -O3)
# add_link_options(-ggdb -O3)

# add_link_options(-fsanitize=address -ggdb)
# add_compile_options(-fsanitize=address -ggdb)

# add_compile_options(-Wall -Wextra -Wpedantic)


## Create main executable

file(GLOB main_src
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
    "src/*/*/*/*.cpp"
    "src/*/*/*/*/*.cpp"
    "src/*/*/*/*/*/*.cpp"
)

add_executable(main 
  ${main_src}
)

target_include_directories(main PRIVATE .)
target_include_directories(main 
  PRIVATE 
    ${STB_INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/source"
    "libs/GeometricTools"
    "libs/GeometricTools/GTE"
)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

target_link_libraries(
  main
  PRIVATE
    freetype
    glfw
    OpenGL::GL
    glad::glad
    glm::glm
    imgui::imgui
  PUBLIC
    OpenMP::OpenMP_CXX
)