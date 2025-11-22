#include "Logger.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL_main.h>

#include <print>

namespace
{
    Earth::Logger s_Logger("Earth");

    SDL_Window* s_Window;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        s_Logger.Error("Failed to initialize SDL Video subsystem: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    s_Window = SDL_CreateWindow("Earth", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!s_Window)
    {
        s_Logger.Error("Failed to create SDL Window: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
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
    SDL_DestroyWindow(s_Window);
}
