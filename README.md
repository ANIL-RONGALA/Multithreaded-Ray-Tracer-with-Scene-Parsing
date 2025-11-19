Multithreaded Ray Tracer with Scene Parsing

A high-performance C++ ray tracing engine with multithreaded rendering and flexible scene description parsing — suitable for research, graphics demos, and reproducible pipelines.

1. Project Overview

This project provides a full ray tracing system built in C++ that:

Parses scene description files (cameras, lights, geometric primitives)

Uses modern math via GLM for vector & matrix operations

Utilizes Boost.Program_options for command-line interface support

Performs multithreaded image rendering for speed

Outputs final renders as 24-bit BMP images

It is intended as a research-grade engine for exploring global illumination, shader models, scene complexity, and parallel performance, with clean modular architecture for extension.

2. Key Features

Multithreading: Efficient rendering across multiple CPU cores

Scene file parsing: Define camera, lights, objects using plain-text scene files

Basic lighting & shading: Support for point lights, diffuse, specular, shadow rays

Vector math abstraction via GLM

CLI configuration of rendering settings via Boost

Output: High resolution renders to BMP format

3. System Requirements & Dependencies

C++17 or later

CMake build system (≥ version 3.10 recommended)

GLM library

Boost (particularly Boost.Program_options)

Threading support (std::thread or equivalent)

A modern CPU with multiple cores for best performance

4. Installation & Build Instructions

Clone the repository:

git clone https://github.com/ANIL-RONGALA/Multithreaded-Ray-Tracer-with-Scene-Parsing.git


Change into project directory:

cd Multithreaded-Ray-Tracer-with-Scene-Parsing


Create build directory and generate build files via CMake:

mkdir build && cd build
cmake ..  


Build the project:

cmake --build . --config Release


Run the executable with a sample scene file:

./raytracer --scene ../scenes/sample.scene --output ../out/render.bmp --width 1920 --height 1080 --threads 8

5. Directory Structure
Multithreaded-Ray-Tracer-with-Scene-Parsing/
│
├── src/                # Engine source code (parser, renderer, threads)  
├── include/            # Header files  
├── scenes/             # Sample scene description files  
├── build/              # CMake build artifacts  
├── out/                # Output renders (BMP images)  
├── CMakeLists.txt      # Build configuration  
└── README.md           # Project documentation (this file)  

6. Usage Guide

Prepare a scene description file (see scenes/ for examples) specifying camera position/rotation, light slots, geometric primitives (spheres, planes, triangle meshes).

Launch the program specifying scene file, output path, image dimensions and number of threads.

The engine will parse the scene, distribute work across threads, compute rays, shading and shadows, then write a .bmp image file.

You can graph performance by varying thread count, resolution, or scene complexity.

7. Extending the Engine

This code base is designed for further research and development. Possible extension areas include:

Add advanced shading models (path tracing, ambient occlusion)

Load external mesh formats (OBJ, FBX)

GPU off-loading (CUDA or Vulkan)

Real-time rendering experiments

Scene complexity benchmarking

Multi-camera & distributed render farms

8. Future Research Applications

Real-time ray tracing on multicore CPUs and heterogeneous systems

Hybrid rendering (CPU + GPU) for academic/industry applications

Scene streaming and dynamic scene updates

Use in visual analytics, architectural simulation, and interactive rendering pipelines

9. Project Name Recommendation

RayTracer-SceneParser-Multithreaded
This repository name is concise, meaningful, and research-friendly, making it suitable for PhD portfolios.

10. Acknowledgements

This engine was developed using standard C++ parallel programming techniques, mathematical libraries (GLM), and command-line utilities (Boost). It reflects my practical skills in systems programming, performance optimization, and software architecture.

Note: A small portion of documentation wording was refined using AI tools for clarity and structure. The design, implementation, algorithms and code logic are entirely original.
