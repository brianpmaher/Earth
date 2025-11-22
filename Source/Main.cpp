#include "Logger.hpp"
#include "Mercator.hpp"
#include "Renderer.hpp"

#include <dotenv.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <print>

namespace
{
    Earth::Logger s_Logger("Earth");

    SDL_Window* s_Window;
    SDL_GLContext s_GLContext;
    std::unique_ptr<Earth::Renderer> s_Renderer;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    dotenv::init();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        s_Logger.Error("Failed to initialize SDL Video subsystem: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    s_Window = SDL_CreateWindow("Earth", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!s_Window)
    {
        s_Logger.Error("Failed to create SDL Window: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    s_GLContext = SDL_GL_CreateContext(s_Window);
    if (!s_GLContext)
    {
        s_Logger.Error("Failed to create OpenGL context: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    s_Renderer = std::make_unique<Earth::Renderer>();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Generate a sphere mesh with 64x64 resolution
    Earth::Mesh sphereMesh = Earth::Mercator::GenerateSphereMesh(64);
    s_Renderer->UploadMesh(sphereMesh);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    int w, h;
    SDL_GetWindowSize(s_Window, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Simple Camera
    float aspectRatio = (float)w / (float)h;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    // Rotate around the Y axis over time
    float time = (float)SDL_GetTicks() / 1000.0f;
    float camX = sin(time) * 3.0f;
    float camZ = cos(time) * 3.0f;

    glm::mat4 view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    s_Renderer->Draw(projection * view);

    SDL_GL_SwapWindow(s_Window);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_ESCAPE)
        {
            return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_GL_DestroyContext(s_GLContext);

    SDL_DestroyWindow(s_Window);
}
