#pragma once

namespace Painter
{
    class Painter
    {
    private:
        /* data */
    public:
        bool renderFPS{false};

        Painter(/* args */);
        ~Painter();

        void customDrawFPS(int posX, int posY);
        void render(int screenWidth, int screenHeight);
    };

} // namespace Painter
