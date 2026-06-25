#include "Painter.h"

#include "raylib.h"

namespace Painter
{
    Painter::Painter(/* args */)
    {
    }

    Painter::~Painter()
    {
    }

    void Painter::render(int screenWidth, int screenHeight)
    {
        BeginDrawing();

        ClearBackground(DARKGRAY);

        DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

        if (renderFPS)
            customDrawFPS(screenWidth - 80, screenHeight - 20);

        EndDrawing();
    }

    void Painter::customDrawFPS(int posX, int posY)
    {
        Color color = ORANGE; // Good FPS
        int fps = GetFPS();

        if ((fps < 30) && (fps >= 15))
            color = YELLOW; // Warning FPS
        else if (fps < 15)
            color = RED; // Low FPS

        DrawText(TextFormat("%2i FPS", fps), posX, posY, 20, color);
    }

} // namespace Painter
