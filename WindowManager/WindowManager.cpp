#include "WindowManager.h"
#include "..\LogManager\LogManager.h"

namespace WindowManager {
    WindowManager::WindowManager() {
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.cbClsExtra = NULL;
        wcex.cbWndExtra = NULL;
        wcex.hInstance = GetModuleHandle(NULL);
        wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

        wcex.style = UINT(WindowStyle::RedrawResize)
            + UINT(WindowStyle::DetectDoubleClicks)
            + UINT(WindowStyle::ModernLook);

        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = nullptr;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hIcon = nullptr;
        wcex.hIconSm = nullptr;

        wcex.lpfnWndProc = StaticWindowProc;

        lastTime = std::chrono::high_resolution_clock::now();
    }
    WindowManager::~WindowManager() {
        if (hWnd != INVALID_HANDLE_VALUE && hWnd != HANDLE(0)) {
            if (IsWindow(hWnd)) {
                if (!DestroyWindow(hWnd)) {
                    LOG_ERROR("WindowManager - ~WindowManager", "Couldn't destroy the window" + to_string(UINT64(hWnd)));
                }
            }
        }
        LPWNDCLASSEXW bin = nullptr;
        if (wcex.lpszClassName != nullptr && GetClassInfoExW(NULL, wcex.lpszClassName, bin)) {
            if (!UnregisterClassW(wcex.lpszClassName, GetModuleHandle(nullptr))) {
                LOG_ERROR(L"WindowManager - CreateWindow", L"Couldn't unregister the class \"" + wstring(wcex.lpszClassName) + L"\"");
            }
        }
    }

    LRESULT CALLBACK WindowManager::StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        WindowManager* window = nullptr;
        if (message == WM_NCCREATE) {
            CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<WindowManager*>(createStruct->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        }
        else {
            window = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        if (window) {
            return window->WindowProc(hWnd, message, wParam, lParam);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT WindowManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_DESTROY:
            LOG_INFO("WindowManager - WindowProc", "Shutting down the window (WM_DESTROY triggerd)");
            isClosing = true;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            break;
        case WM_KEYDOWN:
            if (wParam == VK_F11) {
                EnableFullScreen(!isFullscreen);
            }
            else {
                input->OnKeyDown(wParam);
            }
            break;
        case WM_KEYUP:
            if (wParam != VK_F11) {
                input->OnKeyUp(wParam);
            }
            break;
        case WM_MOUSEMOVE:
            //input->OnMouseMove(LOWORD(lParam), HIWORD(lParam)); // TODO
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    BOOL WindowManager::CreateWindow(const wstring& name) {
        if (wcex.lpszClassName == nullptr) {
            LOG_WARNING(L"WindowManager - CreateWindow", L"wcex.lpszClassName is nullptr, so \"" + name + L"\" is put by default");
            if (!className.empty()) wcex.lpszClassName = className.c_str();
            else LOG_FATAL("WindowManager - CreateWindow", "className and wcex.lpszClassName are empty (probably bescause of dangling pointer)");
        }
        LPWNDCLASSEXW bin = nullptr;
        if (!GetClassInfoExW(NULL, wcex.lpszClassName, bin)) {
            if (RegisterClassExW(&wcex) == INVALID_ATOM) {
                LOG_ERROR(L"WindowManager - CreateWindow", L"Couldn't register the class \"" + wstring(wcex.lpszClassName) + L"\"");
                return false;
            }
        }
        POINT point = { 0 };
        if (!GetCursorPos(&point)) {
            LOG_WARNING("WindowManager - CreateWindow", "Couldn't get the position of the cursor");
            point = { 0 };
        }

        HMONITOR hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (!GetMonitorInfoW(hMonitor, &monitorInfo)) {
            LOG_WARNING("WindowManager - CreateWindow", "Couldn't get the monitor info");
        }

        hWnd = CreateWindowExW(
            WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
            wcex.lpszClassName,
            name.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            monitorInfo.rcWork.left + 100,
            monitorInfo.rcWork.top + 100,
            width,
            height,
            nullptr,
            nullptr, // TODO : MENU HERE
            GetModuleHandle(NULL),
            this);
        if (hWnd == INVALID_HANDLE_VALUE || hWnd == HANDLE(0)) {
            LOG_ERROR("WindowManager - CreateWindow", "Couldn't create a window");
            return false;
        }

        if (!ShowWindow(hWnd, SW_SHOW)) {
            LOG_ERROR("WindowManager - CreateWindow", "Failed to show the window");
        }


        LOG_INFO(L"WindowManager - CreateWindow", L"Starting window : \"" + name + L"\"");
        return true;
    }

    VOID WindowManager::SetWindowTitle(wstring title) {
        if (!hWnd) {
            LOG_ERROR("WindowManager - SetWindowTitle", "hWnd is null");
            return;
        }
        if (!SetWindowTextW(hWnd, title.c_str())) {
            LOG_ERROR("WindowManager - SetWindowTitle", "Couldn't set/change the window title");
        }
    }
    VOID WindowManager::EnableFullScreen(BOOL enabled) {
        DWORD style, exStyle;
        style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

        if (!hWnd) {
            LOG_ERROR("WindowManager - EnableFullScreen", "hWnd is null");
            return;
        }

        if (enabled) {
            RECT windowRect;
            if (!GetWindowRect(hWnd, &windowRect)) LOG_WARNING("WindowManager - EnableFullScreen", "Failed to get window rect (to save position)");
            savedX = windowRect.left;
            savedY = windowRect.top;

            RECT clientRect;
            if (!GetClientRect(hWnd, &clientRect)) LOG_WARNING("WindowManager - EnableFullScreen", "Failed to get client rect (to save width & height)");

            INT borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
            INT borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

            savedWidth = width + borderWidth;
            savedHeight = height + borderHeight;

            style = WS_POPUP | WS_VISIBLE;
            exStyle = WS_EX_APPWINDOW;
        }

        SetWindowLongW(hWnd, GWL_STYLE, style);
        SetWindowLongW(hWnd, GWL_EXSTYLE, exStyle);

        if (enabled) {
            HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monitorInfo{};
            monitorInfo.cbSize = sizeof(monitorInfo);
            if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
#define mpos monitorInfo.rcMonitor
                if (!SetWindowPos(hWnd, nullptr, mpos.left, mpos.top, mpos.right - mpos.left, mpos.bottom - mpos.top, SWP_NOZORDER))
                    LOG_ERROR("WindowManager - EnableFullScreen", "Failed to set window position in fullscreen");
#undef mpos
            }
        }
        else {
            if (!SetWindowPos(hWnd, nullptr, savedX, savedY, savedWidth, savedHeight, SWP_NOZORDER))
                LOG_ERROR("WindowManager - EnableFullScreen", "Failed to set window position in windowed");

            if (!ShowWindow(hWnd, SW_SHOW) || UpdateWindow(hWnd))
                LOG_ERROR("WindowManager - EnableFullScreen", "Failed to show or update window");
        }

        isFullscreen = enabled;
    }
    
    VOID WindowManager::Update() {
        MSG msg = { 0 };

        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                isClosing = true;
                return;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        const auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        input->Update(deltaTime);
    }
    HWND WindowManager::WindowManager::CreateChildWindow(const wstring& nameChild, const POINT pos, const UINT width, const UINT height) {
        if (!hWnd || hWnd == INVALID_HANDLE_VALUE) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Invalid/Null hWnd. Can't create a child window to it");
            return HWND(INVALID_HANDLE_VALUE);
        }
        
        HWND newhWnd = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            wcex.lpszClassName,
            nameChild.c_str(),
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            pos.x,
            pos.y,
            width,
            height,
            hWnd,
            nullptr,
            GetModuleHandle(NULL),
            this);

        if (newhWnd == INVALID_HANDLE_VALUE || newhWnd == HANDLE(0)) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Couldn't create a child window");
            return HWND(INVALID_HANDLE_VALUE);
        }

        if (!ShowWindow(newhWnd, SW_SHOW)) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Failed to show the child window");
        }

        return newhWnd;
    }
    HWND WindowManager::CreateChildWindow(const wstring& nameChild, const FLOAT x, const FLOAT y, const FLOAT width, const FLOAT height) {
        if (!hWnd || hWnd == INVALID_HANDLE_VALUE) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Invalid/Null hWnd. Can't create a child window to it");
            return HWND(INVALID_HANDLE_VALUE);
        }

        RECT clientRect;
        if (!GetClientRect(hWnd, &clientRect)) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Couldn't execute GetClientRect");
            return HWND(INVALID_HANDLE_VALUE);
        }

        INT clientWidth = clientRect.right - clientRect.left;
        INT clientHeight = clientRect.bottom - clientRect.top;

        INT posX = (x <= 1.0f) ? static_cast<INT>(clientWidth * x) : static_cast<INT>(x);
        INT posY = (y <= 1.0f) ? static_cast<INT>(clientHeight * y) : static_cast<INT>(y);

        INT totWidth = (width <= 1.0f) ? static_cast<INT>(clientWidth * width) : static_cast<INT>(width);
        INT totHeight = (height <= 1.0f) ? static_cast<INT>(clientHeight * height) : static_cast<INT>(height);

        posX = std::min(posX, clientWidth - 1);
        posY = std::min(posY, clientHeight - 1);
        totWidth = std::min(totWidth, clientWidth - posX);
        totHeight = std::min(totHeight, clientHeight - posY);

        HWND newhWnd = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            wcex.lpszClassName,
            nameChild.c_str(),
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            posX, posY, totWidth, totHeight,
            hWnd, nullptr, GetModuleHandle(NULL), this);

        if (newhWnd == INVALID_HANDLE_VALUE || newhWnd == HANDLE(0)) {
            LOG_ERROR("WindowManager - CreateChildWindow", "Couldn't create a child window");
            return HWND(INVALID_HANDLE_VALUE);
        }

        LOG_INFO(L"WindowManager - CreateChildWindow", L"Created child window: \"" + nameChild + L"\"");
        return newhWnd;
    }}
