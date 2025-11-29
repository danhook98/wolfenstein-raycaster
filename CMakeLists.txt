cmake_minimum_required(VERSION 3.5)
project(Raycaster)

set(CMAKE_CXX_STANDARD 20)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in deps/SDL
add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

# Add GLEW
# add_subdirectory(deps/glew EXCLUDE_FROM_ALL)

add_executable(Raycaster src/main.cpp
        src/Maths.cpp)

target_include_directories(Raycaster PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Link to the actual SDL3 library.
target_link_libraries(Raycaster PRIVATE SDL3::SDL3)
# target_link_libraries(Raycaster PRIVATE libglew_static)

