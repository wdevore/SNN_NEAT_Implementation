#pragma once

#include "raylib.h"

namespace IOControl
{
    /// @brief Handles keyup/keydown events
    class KeyControl
    {
    private:
        KeyboardKey key{};
        bool keyDown{false};
        bool keyUp{true};
        int state{};
        bool tapped{false};

    public:
        KeyControl(KeyboardKey key) { this->key = key; }
        ~KeyControl();

        bool isTapped();
        bool isActive();
        bool isPressed() { return keyDown; };
        bool isReleased() { return keyUp; }

        void update();
    };

}