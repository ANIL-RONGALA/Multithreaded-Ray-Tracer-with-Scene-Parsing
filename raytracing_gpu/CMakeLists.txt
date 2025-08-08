# Specify the version
cmake_minimum_required(VERSION 3.29)

# Name the project
project(raytrace LANGUAGES CXX CUDA)

# Set C++ standards and build configurations
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set CUDA properties for source files
set_source_files_properties(raytrace.cu PROPERTIES LANGUAGE CUDA)

# Include directories for dependencies
include_directories(
    ${CMAKE_SOURCE_DIR}/include      # Project-specific includes
    ${CMAKE_SOURCE_DIR}/external/glm # glm header-only library
    ${CMAKE_SOURCE_DIR}/external/tira # TIRA library headers
    ${CMAKE_SOURCE_DIR}/external/libjpeg-turbo/include # JPEG library
    ${CMAKE_SOURCE_DIR}/external/libpng/include        # PNG library
    ${CMAKE_SOURCE_DIR}/external/x11/include           # X11 headers
)

# Link directories for dependencies
link_directories(
    ${CMAKE_SOURCE_DIR}/external/tira/lib  # TIRA library
    ${CMAKE_SOURCE_DIR}/external/libjpeg-turbo/lib64 # JPEG library
    ${CMAKE_SOURCE_DIR}/external/libpng/lib          # PNG library
    ${CMAKE_SOURCE_DIR}/external/x11/lib             # X11 libraries
)

# Find and configure Boost
find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost not found! Ensure it is installed and available.")
endif()

# Find and configure CUDA
find_package(CUDA REQUIRED)
if(CUDA_FOUND)
    include_directories(${CUDA_INCLUDE_DIRS})
    link_directories(${CUDA_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "CUDA not found! Ensure it is installed and available.")
endif()

# Add the executable
add_executable(raytrace
    raytrace.cpp
    raytrace.cu
)

# Link libraries
target_link_libraries(raytrace
    ${X11_LIBRARIES}
    Boost::program_options
    jpeg  # Link libjpeg-turbo
    png   # Link libpng
    cuda  # Link CUDA
    tira # Link TIRA library
)