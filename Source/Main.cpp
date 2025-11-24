#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "Logger.hpp"
#include "Mercator.hpp"
#include "Quadtree.hpp"
#include "Renderer.hpp"
#include "ThreadPool.hpp"
#include "TileJSON.hpp"
#include "Tileset.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl3.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_plot.h"

#include <curl/curl.h>
#include <dotenv.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <glm/gtc/matrix_transform.hpp>

#include <format>
#include <memory>
#include <print>

namespace
{
    Earth::Logger s_Logger("Earth");

    struct WindowDeleter
    {
        void operator()(SDL_Window* window) const
        {
            SDL_DestroyWindow(window);
        }
    };

    std::unique_ptr<SDL_Window, WindowDeleter> s_Window;
    SDL_GLContext s_GLContext;
    std::unique_ptr<Earth::Renderer> s_Renderer;
    std::unique_ptr<Earth::Tileset> s_SatelliteTileset;
    std::unique_ptr<Earth::Tileset> s_TerrainTileset;
    std::unique_ptr<Earth::Quadtree> s_Quadtree;
    std::unique_ptr<Earth::Camera> s_Camera;
    std::unique_ptr<Earth::Framebuffer> s_Framebuffer;
    std::unique_ptr<Earth::ThreadPool> s_ThreadPool;
    bool s_ShowLog = false;
    bool s_ShowPerformance = true;
    bool s_ShowLocation = true;
    bool s_ViewportFocused = false;
    bool s_ViewportHovered = false;
    std::vector<float> s_FrameTimes;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    curl_global_init(CURL_GLOBAL_ALL);
    dotenv::init();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        s_Logger.Error("Failed to initialize SDL Video subsystem: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    s_Window.reset(SDL_CreateWindow("Earth", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
    if (!s_Window)
    {
        s_Logger.Error("Failed to create SDL Window: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    s_GLContext = SDL_GL_CreateContext(s_Window.get());
    if (!s_GLContext)
    {
        s_Logger.Error("Failed to create OpenGL context: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    s_Renderer = std::make_unique<Earth::Renderer>();
    s_Camera = std::make_unique<Earth::Camera>(1280.0f, 720.0f);
    s_Framebuffer = std::make_unique<Earth::Framebuffer>(1280, 720);
    s_ThreadPool = std::make_unique<Earth::ThreadPool>(std::thread::hardware_concurrency());

    if (const char* mapTilerKey = std::getenv("MAPTILER_KEY"))
    {
        try
        {
            Earth::URL satUrl =
                std::format("https://api.maptiler.com/tiles/satellite-v2/tiles.json?key={}", mapTilerKey);
            Earth::TileJSON satTileJSON(satUrl);
            auto satTiles = satTileJSON.GetJson()["tiles"];

            Earth::URL terrainUrl =
                std::format("https://api.maptiler.com/tiles/terrain-rgb-v2/tiles.json?key={}", mapTilerKey);
            Earth::TileJSON terrainTileJSON(terrainUrl);
            auto terrainTiles = terrainTileJSON.GetJson()["tiles"];

            if (!satTiles.empty() && !terrainTiles.empty())
            {
                Earth::URL satTileUrl = satTiles[0].get<std::string>();
                Earth::URL terrainTileUrl = terrainTiles[0].get<std::string>();

                s_SatelliteTileset = std::make_unique<Earth::Tileset>(satTileUrl, *s_ThreadPool, true);
                s_TerrainTileset = std::make_unique<Earth::Tileset>(terrainTileUrl, *s_ThreadPool, false);
                s_Quadtree = std::make_unique<Earth::Quadtree>(*s_SatelliteTileset, *s_TerrainTileset);
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

    // Generate a plane mesh with 64x64 resolution (reused for all tiles)
    Earth::Mesh planeMesh = Earth::Mercator::GeneratePlaneMesh(64);
    s_Renderer->UploadMesh(planeMesh);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(s_Window.get(), s_GLContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiID dockSpaceID = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    static bool first_time = true;
    if (first_time)
    {
        first_time = false;

        if (!ImGui::DockBuilderGetNode(dockSpaceID)->IsSplitNode())
        {
            ImGui::DockBuilderRemoveNode(dockSpaceID);
            ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpaceID, ImGui::GetMainViewport()->Size);

            ImGuiID dockMain = dockSpaceID;
            ImGuiID dockLog = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.3f, nullptr, &dockMain);
            ImGuiID dockPerf = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.2f, nullptr, &dockMain);
            ImGuiID dockLoc = ImGui::DockBuilderSplitNode(dockPerf, ImGuiDir_Down, 0.5f, nullptr, &dockPerf);

            ImGui::DockBuilderDockWindow("Viewport", dockMain);
            ImGui::DockBuilderDockWindow("Log", dockLog);
            ImGui::DockBuilderDockWindow("Performance", dockPerf);
            ImGui::DockBuilderDockWindow("Location", dockLoc);
            ImGui::DockBuilderFinish(dockSpaceID);
        }
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                SDL_Event quit_event;
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Log", nullptr, &s_ShowLog))
            {
            }
            if (ImGui::MenuItem("Performance", nullptr, &s_ShowPerformance))
            {
            }
            if (ImGui::MenuItem("Location", nullptr, &s_ShowLocation))
            {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (s_ShowLog)
    {
        Earth::Logger::Draw(&s_ShowLog);
    }

    static float s_TimeAccumulator = 0.0f;

    s_TimeAccumulator += ImGui::GetIO().DeltaTime;

    if (s_TimeAccumulator >= 1.0f)
    {
        if (s_FrameTimes.size() >= 60)
            s_FrameTimes.erase(s_FrameTimes.begin());
        s_FrameTimes.push_back(ImGui::GetIO().Framerate);

        s_TimeAccumulator = 0.0f;
    }

    if (s_ShowPerformance)
    {
        if (ImGui::Begin("Performance", &s_ShowPerformance))
        {
            float framerate = ImGui::GetIO().Framerate;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / framerate, framerate);
            ImGui::Text("Tiles: %d Total, %d Loading, %d Loaded", Earth::Tile::s_TotalTiles.load(),
                        Earth::Tile::s_LoadingTiles.load(), Earth::Tile::s_LoadedTiles.load());

            ImGui::PlotConfig conf;
            conf.values.xs = nullptr;
            conf.values.ys = s_FrameTimes.data();
            conf.values.count = (int)s_FrameTimes.size();
            conf.scale.min = 0;
            conf.scale.max = 144;
            conf.tooltip.show = true;
            conf.tooltip.format = "%.2f FPS";
            conf.grid_x.show = false;
            conf.grid_y.show = true;
            conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 200);
            conf.line_thickness = 2.f;

            ImGui::Plot("FPS", conf);
        }
        ImGui::End();
    }

    if (s_ShowLocation)
    {
        if (ImGui::Begin("Location", &s_ShowLocation))
        {
            // Camera Position (Lon/Lat/Alt)
            float targetLon, targetLat;
            s_Camera->GetTargetLonLat(targetLon, targetLat);
            float range = s_Camera->GetRange();

            float camLonLatAlt[3] = {glm::degrees(targetLon), glm::degrees(targetLat), range};
            ImGui::Text("Camera Lon/Lat/Alt");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat3("##CameraLonLatAlt", camLonLatAlt))
            {
                s_Camera->SetPositionLonLatAlt(glm::radians(camLonLatAlt[0]), glm::radians(camLonLatAlt[1]),
                                               camLonLatAlt[2]);
            }

            // Camera Position (XYZ)
            glm::vec3 camPos = s_Camera->GetPosition();
            float camPosXYZ[3] = {camPos.x, camPos.y, camPos.z};
            ImGui::Text("Camera XYZ");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat3("##CameraXYZ", camPosXYZ, "%.3f"))
            {
                s_Camera->SetPosition(glm::vec3(camPosXYZ[0], camPosXYZ[1], camPosXYZ[2]));
            }

            ImGui::Separator();

            // Look At (Lon/Lat)
            float lookLonLat[2] = {glm::degrees(targetLon), glm::degrees(targetLat)};
            ImGui::Text("Look At Lon/Lat");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat2("##LookAtLonLat", lookLonLat))
            {
                s_Camera->SetTargetLonLat(glm::radians(lookLonLat[0]), glm::radians(lookLonLat[1]));
            }

            // Look At (XYZ)
            glm::vec3 targetPos = s_Camera->GetTargetPosition();
            float targetPosXYZ[3] = {targetPos.x, targetPos.y, targetPos.z};
            ImGui::Text("Look At XYZ");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat3("##LookAtXYZ", targetPosXYZ))
            {
                s_Camera->SetTargetPosition(glm::vec3(targetPosXYZ[0], targetPosXYZ[1], targetPosXYZ[2]));
            }
        }
        ImGui::End();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Viewport"))
    {
        s_ViewportFocused = ImGui::IsWindowFocused();
        s_ViewportHovered = ImGui::IsWindowHovered();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
        {
            s_Framebuffer->Resize((int)viewportPanelSize.x, (int)viewportPanelSize.y);
            s_Camera->Resize(viewportPanelSize.x, viewportPanelSize.y);
        }

        uint64_t textureID = s_Framebuffer->GetTextureID();
        ImGui::Image((ImTextureID)textureID, ImVec2(s_Framebuffer->GetWidth(), s_Framebuffer->GetHeight()),
                     ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
    ImGui::PopStyleVar();

    s_Framebuffer->Bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    s_Camera->Update(0.016f);

    if (s_Quadtree)
    {
        s_Quadtree->Update(*s_Camera);

        glm::mat4 projection = s_Camera->GetProjectionMatrix();
        glm::mat4 view = s_Camera->GetViewMatrix();
        s_Quadtree->Draw(*s_Renderer, projection * view);
    }
    s_Framebuffer->Unbind();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(s_Window.get());

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);

    if (s_Camera)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool handled = false;
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP ||
            event->type == SDL_EVENT_MOUSE_MOTION || event->type == SDL_EVENT_MOUSE_WHEEL)
        {
            if (s_ViewportHovered)
            {
                handled = false;
            }
            else if (io.WantCaptureMouse)
            {
                handled = true;
            }
        }
        else if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP)
        {
            if (s_ViewportFocused)
            {
                handled = false;
            }
            else if (io.WantCaptureKeyboard)
            {
                handled = true;
            }
        }

        if (!handled)
        {
            s_Camera->HandleEvent(*event);
        }
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(s_GLContext);

    s_Window.reset();
    s_ThreadPool.reset();
    curl_global_cleanup();
}
