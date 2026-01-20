# Basilisk Engine

## Building and Running the Project

To build this project from source, you'll use **CMake**. Follow these steps from the command line:

1.  First, navigate to the `build` directory:
    ```bash
    cd build
    ```
2.  Next, run CMake to configure the project. This generates the necessary build files for your system.
    ```bash
    cmake ..
    ```
3.  Then, use CMake to build the project. This compiles the source code and creates the executable file.
    ```bash
    cmake --build .
    ```

Once the build is complete, you can run the final program with this command:

```bash
./render
```

## steps for publishing wheels

```
cmake .. (from build)
cmake --build build (from root)
cmake --install build --prefix ./python
pip install build (run once)
python -m build
```

# Todo

## Rendering
- [x] Lighting System
    - [x] Directional
    - [x] Point
    - [ ] Spot
    - [x] Ambient
- [x] Skybox
- [ ] Shadows
- [ ] Basic PBR
- [ ] Bloom
- [ ] Text Rendering
- [ ] SSAO

## QOL
- [ ] Default lights
- [ ] Default material/texture
- [ ] Material from path
- [ ] Default Mesh

## Optimizations
- [x] Forward+
- [ ] CSM
- [ ] Frustum Culling
- [ ] Auto LOD (meshoptimizer)
- [ ] Instancing

## Physics
