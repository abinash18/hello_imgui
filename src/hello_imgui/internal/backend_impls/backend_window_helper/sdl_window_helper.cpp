#ifdef HELLOIMGUI_USE_SDL

#include "sdl_window_helper.h"
#include "SDL.h"

namespace BackendApi
{
    WindowPointer SdlWindowHelper::CreateWindow(WindowOptions &info)
    {
        int window_flags = 0;

        ScreenBounds &windowBounds = info.windowBounds;

        if (info.fullScreenMode == FullScreenMode::FullMonitorWorkArea)
        {
            auto monitorBounds = GetOneMonitorWorkArea(info.monitorIdx);
            windowBounds = monitorBounds;
        }
        else if (info.fullScreenMode == FullScreenMode::FullScreen)
            window_flags |= SDL_WINDOW_FULLSCREEN;
        else if (info.fullScreenMode == FullScreenMode::FullScreenDesktopResolution)
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

        if (info.backend3DMode == Backend3dMode::OpenGl)
            window_flags |= SDL_WINDOW_OPENGL;
        else if (info.backend3DMode == Backend3dMode::Metal)
            window_flags |= SDL_WINDOW_METAL;
        else if (info.backend3DMode == Backend3dMode::Vulkan)
            window_flags |= SDL_WINDOW_VULKAN;
        else if (info.backend3DMode == Backend3dMode::No3d)
            {}
        else
            BACKEND_THROW("Unsupported backend3DMode");

        if (info.allowHighDpi)
            window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

        if (info.borderless)
            window_flags |= SDL_WINDOW_BORDERLESS;

        if (info.resizable)
            window_flags |= SDL_WINDOW_RESIZABLE;

        if (info.windowSizeState == WindowSizeState::Standard)
            {}
        else if (info.windowSizeState == WindowSizeState::Minimized)
            window_flags |= SDL_WINDOW_MINIMIZED;
        else if (info.windowSizeState == WindowSizeState::Maximized)
            window_flags |= SDL_WINDOW_MAXIMIZED;

        int window_pos[2];
        for (auto dim: Range2)
        {
            if (windowBounds.position[dim] == WindowPositionUnspecified)
                window_pos[dim] = SDL_WINDOWPOS_UNDEFINED;
            else if (windowBounds.position[dim] == WindowPositionCentered)
                window_pos[dim] = SDL_WINDOWPOS_CENTERED;
            else
                window_pos[dim] = windowBounds.position[dim];
        }
        auto window = SDL_CreateWindow(info.windowTitle.c_str(),
                                       window_pos[0], window_pos[1],
                                       windowBounds.size[0], windowBounds.size[1],
                                       window_flags);
        if (!window)
            BACKEND_THROW("BackendSdl::CreateWindow : SDL_CreateWindow returned NULL");

        SDL_GetWindowPosition(window, &windowBounds.position[0], &windowBounds.position[1]);
        SDL_GetWindowSize(window, &windowBounds.size[0], &windowBounds.size[1]);

        return (void *)(window);
    }

    size_t SdlWindowHelper::GetNbMonitors()
    {
        int nbMonitors = SDL_GetNumVideoDisplays();
        return nbMonitors;
    }

    ScreenBounds SdlWindowHelper::GetOneMonitorWorkArea(int monitorIndex)
    {
        assert(monitorIndex >= 0);
        assert(monitorIndex < GetNbMonitors());
        SDL_Rect rect;
        SDL_GetDisplayUsableBounds(monitorIndex, &rect);
        ScreenBounds r{{rect.x, rect.y},
                       {rect.w, rect.h}};
        return r;
    }

    bool SdlWindowHelper::IsWindowIconified(WindowPointer window)
    {
        auto window_flags = SDL_GetWindowFlags((SDL_Window *)(window));
        bool window_is_hidden = (window_flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) > 0;
        return window_is_hidden;
    }

    void SdlWindowHelper::RaiseWindow(WindowPointer window)
    {
        // Despite those efforts, the app does not come to the front under MacOS
        auto sdlWindow = (SDL_Window *)(window);
        SDL_ShowWindow(sdlWindow);
        SDL_FlashWindow(sdlWindow, SDL_FLASH_UNTIL_FOCUSED);
        SDL_RaiseWindow(sdlWindow);
    }

    ScreenBounds SdlWindowHelper::GetWindowBounds(WindowPointer window)
    {
        auto sdlWindow = (SDL_Window *)(window);
        int x, y, w, h;
        SDL_GetWindowSize(sdlWindow, &w, &h);
        SDL_GetWindowPosition(sdlWindow, &x, &y);
        ScreenBounds r{{x, y}, {w, h}};
        return r;
    }

    void SdlWindowHelper::SetWindowBounds(WindowPointer window, ScreenBounds windowBounds)
    {
        auto sdlWindow = (SDL_Window *)(window);

        // Size
        SDL_SetWindowSize(sdlWindow, windowBounds.size[0], windowBounds.size[1]);

        // Position
        if (windowBounds.position[0] == WindowPositionCentered)
            {}
        else if (windowBounds.position[0] == WindowPositionUnspecified)
            {}
        else
            SDL_SetWindowPosition(sdlWindow, windowBounds.position[0], windowBounds.position[1]);
    }

} // namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
