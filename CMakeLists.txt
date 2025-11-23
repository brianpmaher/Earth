cmake_minimum_required(VERSION 3.30)

project(Earth VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CPM_FILE_PATH ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

include(cmake/get_cpm.cmake)

CPMAddPackage(
    NAME SDL
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-3.2.26
)

CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 1.0.2
)

CPMAddPackage(
    NAME dotenv-cpp
    GITHUB_REPOSITORY laserpants/dotenv-cpp
    GIT_TAG master
)

CPMAddPackage(
    NAME json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.12.0
)

CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG master
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME libwebp
    GITHUB_REPOSITORY webmproject/libwebp
    GIT_TAG v1.3.2
)

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG docking
)

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    GIT_TAG v1.14.1
)

CPMAddPackage(
    NAME imgui_plot
    GITHUB_REPOSITORY soulthreads/imgui-plot
    GIT_TAG master
    DOWNLOAD_ONLY YES
)

find_package(CURL REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(${PROJECT_NAME}
    Source/Logger.cpp
    Source/Renderer.cpp
    Source/Shader.cpp
    Source/Camera.cpp
    Source/Quadtree.cpp
    Source/Mercator.cpp
    Source/TileJSON.cpp
    Source/Tileset.cpp
    Source/HTTP.cpp
    Source/Image.cpp
    Source/Main.cpp
    Source/Framebuffer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${stb_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_plot_SOURCE_DIR}/include
)

target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_plot_SOURCE_DIR}/src/imgui_plot.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    glm::glm
    dotenv
    nlohmann_json::nlohmann_json
    CURL::libcurl
    webp
    spdlog::spdlog
    "-framework OpenGL"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SDL_MAIN_USE_CALLBACKS
    GL_SILENCE_DEPRECATION
)
