#pragma once
#include "include.h"

namespace Input {
    enum class Key {
        A = 'A',
        B = 'B',
        C = 'C',
        D = 'D',
        E = 'E',
        F = 'F',
        G = 'G',
        H = 'H',
        I = 'I',
        J = 'J',
        K = 'K',
        L = 'L',
        M = 'M',
        N = 'N',
        O = 'O',
        P = 'P',
        Q = 'Q',
        R = 'R',
        S = 'S',
        T = 'T',
        U = 'U',
        V = 'V',
        W = 'W',
        X = 'X',
        Y = 'Y',
        Z = 'Z',
        
        n0 = '0',
        n1 = '1',
        n2 = '2',
        n3 = '3',
        n4 = '4',
        n5 = '5',
        n6 = '6',
        n7 = '7',
        n8 = '8',
        n9 = '9',

        Escape = VK_ESCAPE,
        Enter = VK_RETURN,

        Space = VK_SPACE,
        Shift = VK_SHIFT,
        Ctrl = VK_CONTROL,

        Left = VK_LEFT,
        Right = VK_RIGHT,
        Up = VK_UP,
        Down = VK_DOWN,

        Mouse_Left = VK_LBUTTON,
        Mouse_Right = VK_RBUTTON,
        Mouse_Middle = VK_MBUTTON,
        
        F1 = VK_F1,
        F2 = VK_F2,
        F3 = VK_F3,
        F4 = VK_F4,
        F5 = VK_F5,
        F6 = VK_F6,
        F7 = VK_F7,
        F8 = VK_F8,
        F9 = VK_F9,
        F10 = VK_F10,
        F11 = VK_F11,
        F12 = VK_F12,
    };
    enum class KeyAction {
        SimplePress,
        DoublePress,
        Holding,
    };
    
    class Input {
    public:
        Input() {}
        Input(HWND hWnd) : hwnd(hWnd) {}
        ~Input();

        void Update(const FLOAT deltaTime);


        // Keyboard (mostly)
        void OnKeyDown(WPARAM key);
        void OnKeyUp(WPARAM key);
        void MapAction(const std::string& actionName, Key key, std::function<void()> callback, KeyAction execution, BOOL conflict = false);
        void MapAction(const std::string& actionName, const std::vector<Key>& keys, std::function<void()> callback, KeyAction execution, BOOL conflict = false);
        void UnmapAction(const std::string& actionName);
        
        bool IsKeyHeld(Key key) const;
        bool IsKeyPressed(Key key) const;
        bool IsKeyDoublePressed(Key key) const;
        
        // Other
        void OnMouseMove(int x, int y);
        void OnMouseWheel(int delta);
        void MapMouse(const std::string& actionName, std::function<void()> callback);
        void UnmapMouse(const std::string& actionName);

        POINT GetMousePosition() const { return mousePos; }
        POINT GetMouseDelta() const { return mouseDelta; }
        int GetMouseWheelDelta() const { return mouseWheelDelta; }

        void MapAxis(const std::string& axisName, Key positiveKey, Key negativeKey, std::function<void(float)> callback);
        void UnmapAxis(const std::string& axisName);

        constexpr float GetDeltaHorizontal() { return static_cast<float>(mouseDelta.x); }
        constexpr float GetDeltaVertical() const { return static_cast<float>(mouseDelta.y); }

        void SetCenterWindow(POINT center) { windowCenter = center; }
        void SetMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
        void SetMouseLocked(bool locked);
        void RecenterMouse();
        bool IsMouseInWindow();
        bool IsMouseLocked() const { return mouseLocked; }
        bool IsPressOnWindow();
        inline bool IsMouseLocked() { return mouseLocked; }
        float GetSmoothDeltaHorizontal() const {
            return smoothedDeltaX * mouseSensitivity;
        }
        float GetSmoothDeltaVertical() const {
            return smoothedDeltaY * mouseSensitivity;
        }
        void SetMouseSmoothingFactor(float factor) {
            mouseSmoothingFactor = std::clamp(factor, 0.0f, 1.0f);
        }
        void SetMouseHistorySize(size_t size) {
            mouseHistorySize = std::max(size, static_cast<size_t>(1));
            if (mouseHistoryX.size() > mouseHistorySize) {
                mouseHistoryX.resize(mouseHistorySize);
                mouseHistoryY.resize(mouseHistorySize);
            }
        }

        void SetHWND(HWND hWnd) { hwnd = hWnd; }
    private:
        // Keyboard (mostly)
        static constexpr FLOAT TAP_THRESHOLD = std::chrono::duration<float>(std::chrono::milliseconds(100)).count();
        static constexpr FLOAT DOUBLE_TAP_THRESHOLD = std::chrono::duration<float>(std::chrono::milliseconds(300)).count();
        static constexpr FLOAT DEFAULT_CONFLICT_DELAY = std::chrono::duration<float>(std::chrono::milliseconds(200)).count();


        struct InputAction {
            std::vector<Key> keys;
            std::function<void()> callback;
            KeyAction execution;
            BOOL needsConflictResolution = false;
            BOOL executed = false;
        };
        std::unordered_map<std::string, InputAction> actions;
        struct PendingCallback {
            std::function<void()> callback;
            Key key;
            FLOAT delay;
            FLOAT timer = 0.0f;
        };
        std::vector<PendingCallback> pendingCallback;
        void ProcessActions(const FLOAT deltaTime);
        inline void PushCallback(std::vector<std::function<void()>>& vectorCallback, std::function<void()> callback);

        struct KeyState {
            enum class State {
                Pressed,
                DoublePressed,
                Held,
                Released,
                Cooldown
            } state;
            FLOAT timer = 0.f;
        };
        std::unordered_map<Key, KeyState> keyStates;
        void UpdateKeyStates(const FLOAT deltaTime);
        
        // Other
        struct MouseAction {
            std::string name;
            std::function<void()> callback;
        };
        struct InputAxis {
            std::string name;
            Key positiveKey;
            Key negativeKey;
            std::function<void(float)> callback;
            float deadzone = 0.1f;
            float sensitivity = 1.0f;
        };


        HWND hwnd = nullptr;
        POINT windowCenter = { 0, 0 };
        
        
        std::unordered_map<std::string, MouseAction> mouseActions;
        std::unordered_map<std::string, InputAxis> axes;
        
        POINT mousePos = { 0, 0 };
        POINT mouseDelta = { 0, 0 };
        POINT lastMousePos = { 0, 0 };
        int mouseWheelDelta = 0;
        float mouseSensitivity = 0.2f;
        bool mouseLocked = false;
        RECT originalClipRect;
        
        
        void ProcessMouseActions();
        void ProcessAxes(const float deltaTime);
        void UpdateMouseDelta();
        constexpr void PCUpdateMouseDelta(POINT&);
        
        bool shouldRecenterMouse = false;
        float rawMouseDeltaX = 0.0f;
        float rawMouseDeltaY = 0.0f;
        float smoothedDeltaX = 0.0f;
        float smoothedDeltaY = 0.0f;
        float mouseSmoothingFactor = .3f;
        std::vector<float> mouseHistoryX;
        std::vector<float> mouseHistoryY;
        size_t mouseHistorySize = 3;
        bool centerNextFrame = false;
    };


}
using Key = Input::Key;
using KeyAction = Input::KeyAction;
