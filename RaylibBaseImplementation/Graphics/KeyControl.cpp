#include "KeyControl.h"

namespace IOControl
{

    KeyControl::~KeyControl()
    {
    }

    bool KeyControl::isTapped()
    {
        if (tapped)
        {
            tapped = false;
            return true;
        }
        return false;
    }

    bool KeyControl::isActive()
    {
        return keyDown;
    }

    void KeyControl::update()
    {
        keyDown = IsKeyDown(key);
        keyUp = IsKeyUp(key);

        if (keyDown && state == 0)
        {
            state = 1;
        }
        if (keyUp && state == 1)
        {
            tapped = true;
            state = 0;
        }
    }
}