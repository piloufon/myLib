#include "Input.h"
#include "..\LogManager\LogManager.h"


using std::vector;
using std::string;
using std::wstring;
using std::to_string;
using std::to_wstring;
using std::move;

namespace Input {
    Input::~Input() {
        if (mouseLocked) {
            SetMouseLocked(false);
        }
        axes.clear();
        actions.clear();
        keyStates.clear();
    }

    void Input::Update(const FLOAT deltaTime) {
        UpdateKeyStates(deltaTime);
        ProcessActions(deltaTime);
        UpdateMouseDelta();
        ProcessMouseActions();
        ProcessAxes(deltaTime);

        if (mouseLocked) {
            POINT currentPos;
            GetCursorPos(&currentPos);

            int distanceFromCenter = abs(currentPos.x - windowCenter.x) + abs(currentPos.y - windowCenter.y);

            if (distanceFromCenter > 100) {
                centerNextFrame = true;
                SetCursorPos(windowCenter.x, windowCenter.y);
            }
        }

        mouseWheelDelta = 0;
    }


    // Keyboard (mostly)
    void Input::MapAction(const std::string& actionName, const vector<Key>& keys, std::function<void()> callback, KeyAction execution, BOOL conflict) {
        actions[actionName] = { keys, callback, execution, conflict };
        for (Key key : keys) {
            keyStates[key].state = KeyState::State::Released;
        }
    }
    void Input::MapAction(const std::string& actionName, Key key, std::function<void()> callback, KeyAction execution, BOOL conflict) {
        actions[actionName] = { {key}, callback, execution, conflict };
        keyStates[key].state = KeyState::State::Released;
    }
    void Input::UnmapAction(const std::string& actionName) {
        actions.erase(actionName);
    }

    bool Input::IsKeyHeld(Key key) const {
        auto it = keyStates.find(key);
        return it != keyStates.end() && it->second.state == KeyState::State::Held;
    }
    bool Input::IsKeyPressed(Key key) const {
        auto it = keyStates.find(key);
        return it != keyStates.end() && it->second.state == KeyState::State::Pressed;
    }
    bool Input::IsKeyDoublePressed(Key key) const {
        auto it = keyStates.find(key);
        return it != keyStates.end() && it->second.state == KeyState::State::DoublePressed;
    }

    void Input::OnKeyDown(WPARAM key) {
        Key keyEnum = static_cast<Key>(key);
        if (keyStates.find(keyEnum) != keyStates.end()) {
            if (keyStates[keyEnum].state == KeyState::State::Released) keyStates[keyEnum].state = KeyState::State::Pressed;
            else if (keyStates[keyEnum].state == KeyState::State::Cooldown) {
                keyStates[keyEnum].state = KeyState::State::DoublePressed;
                keyStates[keyEnum].timer = 0.f;
            }
        }
    }
    void Input::OnKeyUp(WPARAM key) {
        Key keyEnum = static_cast<Key>(key);
        if (keyStates.find(keyEnum) != keyStates.end()) {
            if (keyStates[keyEnum].state == KeyState::State::Pressed) {
                keyStates[keyEnum].state = KeyState::State::Cooldown;
            }
            else keyStates[keyEnum].state = KeyState::State::Released;
            keyStates[keyEnum].timer = 0.f;
        }
    }

    inline void Input::PushCallback(std::vector<std::function<void()>>& vectorCallback, std::function<void()> callback) {
        if (callback) [[likely]] {
            vectorCallback.push_back(callback);
        }
        else [[unlikely]] {
            LOG_WARNING(L"Input - PushCallBack", L"The callback is empty");
        }
    }
    void Input::ProcessActions(const FLOAT deltaTime) {
        vector<std::function<void()>> callbacksToExecute;

        for (auto it = pendingCallback.begin(); it != pendingCallback.end();) {
            it->timer += deltaTime;
            if (it->timer >= it->delay) {
                callbacksToExecute.push_back(it->callback);
                it = pendingCallback.erase(it);
            }
            else {
                ++it;
            }
        }

        for (auto& [name, action] : actions) {
            if (action.execution == KeyAction::Holding) {
                bool anyKeyActive = false;

                for (Key key : action.keys) {
                    if (IsKeyHeld(key) || IsKeyPressed(key)) {
                        anyKeyActive = true;

                        PushCallback(callbacksToExecute, action.callback);
                        break;
                    }
                }
            }
            else if (action.execution == KeyAction::SimplePress) {
                bool anyKeyActive = false;

                for (Key key : action.keys) {
                    if (IsKeyPressed(key) || IsKeyDoublePressed(key) || IsKeyHeld(key)) {
                        anyKeyActive = true;
                        if (!action.executed) {
                            if (action.needsConflictResolution) {
                                if (IsKeyPressed(key) && !IsKeyDoublePressed(key)) {
                                    pendingCallback.push_back({ action.callback, key, DEFAULT_CONFLICT_DELAY, 0.0f });
                                    action.executed = true;
                                }
                            }
                            else {
                                PushCallback(callbacksToExecute, action.callback);
                                action.executed = true;
                            }
                        }
                        break;
                    }
                }

                if (!anyKeyActive) {
                    action.executed = false;
                }
            }
            else if (action.execution == KeyAction::DoublePress) {
                bool anyKeyActive = false;

                for (Key key : action.keys) {
                    if (IsKeyDoublePressed(key)) {
                        anyKeyActive = true;

                        if (!action.executed) {
                            PushCallback(callbacksToExecute, action.callback);
                            action.executed = true;

                            for (auto it = pendingCallback.begin(); it != pendingCallback.end();) {
                                if (it->key == key) {
                                    it = pendingCallback.erase(it);
                                }
                                else {
                                    it++;
                                }
                            }
                        }
                        break;
                    }
                }

                if (!anyKeyActive) {
                    action.executed = false;
                }
            }
        }

        for (auto& callback : callbacksToExecute) {
            callback();
        }
    }
    void Input::Input::UpdateKeyStates(const FLOAT deltaTime) {
        for (auto& [key, state] : keyStates) {
            switch (state.state) {
            case KeyState::State::Pressed:
                state.timer += deltaTime;
                if (state.timer >= TAP_THRESHOLD) state.state = KeyState::State::Held;
                break;

            case KeyState::State::Held:
                state.timer += deltaTime;
                break;

            case KeyState::State::Cooldown:
                state.timer += deltaTime;
                if (state.timer >= DOUBLE_TAP_THRESHOLD) {
                    state.state = KeyState::State::Released;
                    state.timer = 0.f;
                }
                break;
            }
        }
    }

    // Other

    void Input::RecenterMouse() {
        shouldRecenterMouse = true;
        SetCursorPos(windowCenter.x, windowCenter.y);
    }
    bool Input::IsMouseInWindow() {
        if (!hwnd) return false;

        POINT cursorPos;
        GetCursorPos(&cursorPos);
        HWND windowUnderCursor = WindowFromPoint(cursorPos);

        return (windowUnderCursor == hwnd || IsChild(hwnd, windowUnderCursor));
    }
    bool Input::IsPressOnWindow() {
        if (!IsMouseInWindow()) return false;
        bool isLeftMousePressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        return isLeftMousePressed;
    }

    void Input::UnmapAxis(const std::string& axisName) {
        axes.erase(axisName);
    }
    void Input::ProcessMouseActions() {
        for (const auto& [name, action] : mouseActions) {
            if (IsMouseInWindow() && action.callback) {
                action.callback();
            }
        }
    }
    void Input::ProcessAxes(const float deltaTime) {
        for (const auto& [name, axis] : axes) {
            float value = 0.0f;

            if (IsKeyHeld(axis.positiveKey) || IsKeyPressed(axis.positiveKey)) {
                value += 1.0f;
            }
            if (IsKeyHeld(axis.negativeKey) || IsKeyPressed(axis.negativeKey)) {
                value -= 1.0f;
            }

            if (abs(value) > axis.deadzone) {
                value *= axis.sensitivity * deltaTime;
                if (axis.callback) {
                    axis.callback(value);
                }
            }
        }
    }
    void Input::UpdateMouseDelta() {
        POINT currentPos;
        GetCursorPos(&currentPos);

        PCUpdateMouseDelta(currentPos);
    }

    constexpr void Input::PCUpdateMouseDelta(POINT& currentPos) {
        if (shouldRecenterMouse || centerNextFrame) {
            shouldRecenterMouse = false;
            centerNextFrame = false;
            lastMousePos = currentPos;
            rawMouseDeltaX = 0.0f;
            rawMouseDeltaY = 0.0f;
            return;
        }

        rawMouseDeltaX = static_cast<float>(currentPos.x - lastMousePos.x);
        rawMouseDeltaY = static_cast<float>(currentPos.y - lastMousePos.y);

        mouseHistoryX.push_back(rawMouseDeltaX);
        mouseHistoryY.push_back(rawMouseDeltaY);

        if (mouseHistoryX.size() > mouseHistorySize) {
            mouseHistoryX.erase(mouseHistoryX.begin());
            mouseHistoryY.erase(mouseHistoryY.begin());
        }

        float avgDeltaX = 0.0f;
        float avgDeltaY = 0.0f;
        for (size_t i = 0; i < mouseHistoryX.size(); ++i) {
            avgDeltaX += mouseHistoryX[i];
            avgDeltaY += mouseHistoryY[i];
        }
        avgDeltaX /= static_cast<float>(mouseHistoryX.size());
        avgDeltaY /= static_cast<float>(mouseHistoryY.size());

        smoothedDeltaX = smoothedDeltaX * mouseSmoothingFactor + avgDeltaX * (1.0f - mouseSmoothingFactor);
        smoothedDeltaY = smoothedDeltaY * mouseSmoothingFactor + avgDeltaY * (1.0f - mouseSmoothingFactor);

        mouseDelta.x = static_cast<LONG>(smoothedDeltaX);
        mouseDelta.y = static_cast<LONG>(smoothedDeltaY);

        lastMousePos = currentPos;
        mousePos = currentPos;
    }

    void Input::MapAxis(const std::string& axisName, Key positiveKey, Key negativeKey, std::function<void(float)> callback) {
        axes[axisName] = { axisName, positiveKey, negativeKey, callback };
        keyStates[positiveKey] = { KeyState::State::Released, 0.0f };
        keyStates[negativeKey] = { KeyState::State::Released, 0.0f };
    }
    void Input::MapMouse(const std::string& actionName, std::function<void()> callback) {
        mouseActions[actionName] = { actionName, callback };
    }
    void Input::UnmapMouse(const std::string& actionName) {
        mouseActions.erase(actionName);
    }
    void Input::SetMouseLocked(bool locked) {
        if (locked && !mouseLocked) {
            GetClipCursor(&originalClipRect);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            ClientToScreen(hwnd, reinterpret_cast<POINT*>(&clientRect.left));
            ClientToScreen(hwnd, reinterpret_cast<POINT*>(&clientRect.right));
            ClipCursor(&clientRect);

            ShowCursor(FALSE);

            shouldRecenterMouse = true;
            SetCursorPos(windowCenter.x, windowCenter.y);
        }
        else if (!locked && mouseLocked) {
            ClipCursor(&originalClipRect);
            ShowCursor(TRUE);
        }

        mouseLocked = locked;
    }
    void Input::OnMouseMove(int x, int y) {
        mousePos.x = x;
        mousePos.y = y;
    }
    void Input::OnMouseWheel(int delta) {
        mouseWheelDelta = delta;
    }
}
