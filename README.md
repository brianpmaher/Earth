# Earth

A 3D Earth renderer with satellite imagery and terrain elevation data fetched dynamically from MapTiler.

![](Screenshot.png)

## Prerequisites

Before building the project, ensure you have the following installed:

-   **C++ Compiler**: A compiler with C++23 support (e.g., Clang 16+, GCC 13+, MSVC 2022).
-   **CMake**: Version 3.30 or later.
-   **Ninja**: Recommended build system (optional).
-   **libcurl**: Required for HTTP requests.
-   **MapTiler API Key**: You need a free API key from [MapTiler](https://www.maptiler.com/).

## Setup

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/brianpmaher/Earth.git
    cd Earth
    ```

2.  **Configure Environment Variables**:
    Create a `.env` file in the root directory of the project and add your MapTiler API key:
    ```env
    MAPTILER_KEY=your_maptiler_api_key_here
    ```

## Build

Use CMake to configure and build the project.

```bash
# Configure the project (using Ninja)
cmake -S . -B Build -G Ninja

# Build the project
cmake --build Build
```

## Run

After building, run the executable from the `Build` directory:

```bash
./Build/Debug/Earth
```

## Controls

| Input | Action |
| :--- | :--- |
| **Left Mouse Drag** | Pan the camera (move around). |
| **Right Mouse Drag** | Rotate and Tilt the camera. |
| **Mouse Wheel** | Zoom in and out. |
| **Escape** | Quit the application. |

## Dependencies

This project uses the following libraries, managed via CPM.cmake:

-   [SDL3](https://github.com/libsdl-org/SDL): Windowing, Input, and OpenGL context creation.
-   [glm](https://github.com/g-truc/glm): Mathematics library for graphics software.
-   [dotenv-cpp](https://github.com/laserpants/dotenv-cpp): Loads environment variables from `.env` files.
-   [nlohmann_json](https://github.com/nlohmann/json): JSON for Modern C++.
-   [stb](https://github.com/nothings/stb): Image loading (stb_image).
-   [libwebp](https://github.com/webmproject/libwebp): WebP image decoding.
-   [libcurl](https://curl.se/libcurl/): Client-side URL transfer library.
