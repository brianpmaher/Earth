#include "Camera.hpp"
#include "Logger.hpp"
#include "Mercator.hpp"
#include "Renderer.hpp"
#include "TileJSON.hpp"
#include "Tileset.hpp"

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
    std::shared_ptr<Earth::Tile> s_Tile;
    std::unique_ptr<Earth::Camera> s_Camera;
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
    s_Camera = std::make_unique<Earth::Camera>(1280.0f, 720.0f);

    if (const char* mapTilerKey = std::getenv("MAPTILER_KEY"))
    {
        try
        {
            Earth::URL url = std::format("https://api.maptiler.com/tiles/satellite-v2/tiles.json?key={}", mapTilerKey);
            Earth::TileJSON tileJSON(url);
            auto tiles = tileJSON.GetJson()["tiles"];
            if (!tiles.empty())
            {
                Earth::URL tileUrl = tiles[0].get<std::string>();
                Earth::Tileset tileset(tileUrl);
                s_Tile = tileset.LoadTile(0, 0, 0);
            }
        }
        catch (const std::exception& e)
        {
            s_Logger.Error("Failed to fetch tileset: {}", e.what());
        }
    }
    else
    {
        s_Logger.Error("MAPTILER_KEY not set in .env");
    }

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
    s_Camera->Resize((float)w, (float)h);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (s_Tile)
    {
        s_Tile->Bind(0);
    }

    s_Camera->Update(0.016f); // Fixed time step for now

    glm::mat4 projection = s_Camera->GetProjectionMatrix();
    glm::mat4 view = s_Camera->GetViewMatrix();

    s_Renderer->Draw(projection * view, true);

    SDL_GL_SwapWindow(s_Window);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (s_Camera)
    {
        s_Camera->HandleEvent(*event);
    }

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
