#include "Application.hpp"
#include "Logger.hpp"

#include <SDL3/SDL_main.h>

#include <print>

namespace Earth
{
    struct AppState
    {
        Logger logger{"SDLApp"};
        Application application;
    };
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    *appstate = new Earth::AppState();

    auto* state = static_cast<Earth::AppState*>(*appstate);
    state->logger.Info("Application initializing with {} arguments.", argc);

    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* state = static_cast<Earth::AppState*>(appstate);
    state->logger.Info("Application quitting with result: {}", static_cast<int>(result));

    delete state;
}
