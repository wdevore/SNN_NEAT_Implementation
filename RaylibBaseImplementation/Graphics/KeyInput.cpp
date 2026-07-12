#include <iostream>

#include "KeyInput.h"

namespace IOControl
{
    KeyInput::KeyInput(/* args */)
    {
    }

    KeyInput::~KeyInput()
    {
    }

    void KeyInput::scan()
    {
        keyS.update();
        keyF.update();
        keyW.update();
        keyX.update();
        keyL.update();
        keyP.update();
        keyR.update();
        keyE.update();
        keyH.update();
        keyI.update();
        keyO.update();

        key1.update();
        key2.update();
        key3.update();
        key4.update();
        key5.update();
        key6.update();
    }

    void KeyInput::process(Painter::Painter &painter)
    {
        if (keyW.isActive())
        {
            std::cout << "W pressed" << std::endl;
        }
        if (IsKeyDown(KEY_A))
        {
        }
        if (IsKeyDown(KEY_D))
        {
        }

        if (IsKeyDown(KEY_UP))
        {
        }
        if (IsKeyDown(KEY_DOWN))
        {
        }
        if (IsKeyDown(KEY_LEFT))
        { /* Rotate Left */
        }
        if (IsKeyDown(KEY_RIGHT))
        { /* Rotate Right */
        }

        if (IsKeyPressed(KEY_SPACE))
        { /* Toggle Animation/Action */
        }

        if (keyX.isTapped())
        {
            // std::cout << "Backface culling Enabled: " << (pipeline.shouldCullBackfaces ? "Yes" : "No") << std::endl;
        }
        if (keyL.isTapped())
        {
        }

        if (keyF.isTapped())
        {
        }

        if (keyP.isTapped())
        {
            painter.renderFPS = !painter.renderFPS;
        }

        if (key1.isTapped())
        {
        }

        if (key2.isTapped())
        {
        }

        if (key3.isTapped())
        {
        }

        if (key4.isTapped())
        {
        }

        if (key5.isTapped())
        {
        }

        if (key6.isTapped())
        {
        }

        if (keyH.isTapped())
        {
            std::cout << "======================================" << std::endl;
            std::cout << "R = Reset" << std::endl;
            std::cout << "S = Step" << std::endl;
            std::cout << "P = Toggle FPS" << std::endl;
            std::cout << "H = Help" << std::endl;
            std::cout << "` = Quit" << std::endl;
            std::cout << "======================================" << std::endl;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            mouseDown = true;
            // pipeline.OnMouseDown(GetMouseX(), GetMouseY()); // For Arcball
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            mouseDown = false;
            // pipeline.OnMouseUp(); // For Arcball
        }

        // Check if the mouse moved this frame
        Vector2 mouseDelta = GetMouseDelta();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            if (mouseDown)
            {
                //     std::cout << "Mouse moved: " << mouseDelta.x << ", " << mouseDelta.y << std::endl;
                //     std::cout << "Mouse position: " << GetMouseX() << ", " << GetMouseY() << std::endl;
                // pipeline.OnMouseMove(GetMouseX(), GetMouseY(), (int)mouseDelta.x, (int)mouseDelta.y);
            }
            else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
            {
                // pipeline.OnMousePan(GetMouseX(), GetMouseY(), (int)mouseDelta.x, (int)mouseDelta.y);
            }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            // pipeline.OnMouseWheel(wheel);
        }
    }
}