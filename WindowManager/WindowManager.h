#pragma once
#include "include.h"

namespace WindowManager {
#ifdef CreateWindow
    #undef CreateWindow
#endif
#ifdef VOID
    #undef VOID
    using VOID = void;
#endif

    class WindowManager {
    public:
        enum class WindowStyle {
            RedrawResize = CS_HREDRAW | CS_VREDRAW,
            DetectDoubleClicks = CS_DBLCLKS,
            OwnDeviceContext = CS_OWNDC, // Test if compatible with D3D12
            ModernLook = CS_DROPSHADOW
        };

        WindowManager();
        ~WindowManager();
        
        VOID SetWndClass(WNDCLASSEX newWcex) { wcex = newWcex; }
        VOID SetStyle(UINT style) { wcex.style = style; }
        VOID SetCursor(HCURSOR cursor) { wcex.hCursor = cursor; }
        VOID SetIcon(HICON icon) { wcex.hIcon = icon; wcex.hIconSm = icon; }
        VOID SetWindowProc(WNDPROC customWinPro) { wcex.lpfnWndProc = customWinPro; }
        VOID SetMenuName(wstring menuName) { wcex.lpszMenuName = menuName.c_str(); }
        VOID SetClassName(wstring className) { this->className = className;  wcex.lpszClassName = this->className.c_str(); }

        static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        BOOL CreateWindow(const wstring& name);
        VOID SetWindowTitle(wstring title);

        VOID SetInput(Input::Input* newInput) { input = newInput; }
        Input::Input* GetpInput() { return input; }

        VOID Update();

        constexpr UINT32 GetWidth() { return width; }
        constexpr UINT32 GetHeight() { return height; }

        constexpr BOOL ShouldClose() { return !isClosing; }
        constexpr FLOAT GetDeltaTime() { return deltaTime; }
        
        [[nodiscard]] HWND CreateChildWindow(const wstring& nameChild, const POINT pos, const UINT width, const UINT height);
        [[nodiscard]] HWND CreateChildWindow(const wstring& nameChild, const FLOAT x, const FLOAT y, const FLOAT width, const FLOAT height);

        [[nodiscard]] constexpr HWND GetHWnd() { return hWnd; }
    private:
        BOOL isClosing = false;

        wstring className = L"defaultClassName";
        WNDCLASSEX wcex;
        HWND hWnd = { 0 };


        BOOL isFullscreen = false;
  
        INT width  = 16 * 60;
        INT height =  9 * 60;

        INT savedWidth   = 0;
        INT savedHeight  = 0;
        INT savedX       = 0;
        INT savedY       = 0;


        Input::Input* input = nullptr;

        VOID EnableFullScreen(BOOL enable);

        std::chrono::high_resolution_clock::time_point lastTime;
        FLOAT deltaTime = 0.f;
    };

}
