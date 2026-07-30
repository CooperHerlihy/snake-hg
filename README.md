# Snake HG

## Description

Snake, written to test basic usage of Hurdy Gurdy 2D

## Build

Dependencies:
- CMake
- Ninja
- Optional: Mold
- Optional: CCache
- C++ compiler (GCC, Clang, or msvc)
- Optional: SDL3 (for hurdygurdy, can compile from scratch if not found)
- Linux:
    - GLSL compiler (glslc)
    - Vulkan Validation Layers (for hurdygurdy debug mode)
- Windows:
    - Lunarg Vulkan SDK (includes glslc and validation layers)

To build, run cmake:

```bash
cmake --workflow --preset debug
```

