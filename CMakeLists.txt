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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(${PROJECT_NAME}
    Source/Logger.cpp
    Source/Renderer.cpp
    Source/Mercator.cpp
    Source/Main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    glm::glm
    "-framework OpenGL"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SDL_MAIN_USE_CALLBACKS
)
