#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

#include "raylib.h"

#include "KeyInput.h"

void CustomDrawFPS(int posX, int posY)
{
  Color color = ORANGE; // Good FPS
  int fps = GetFPS();

  if ((fps < 30) && (fps >= 15))
    color = YELLOW; // Warning FPS
  else if (fps < 15)
    color = RED; // Low FPS

  DrawText(TextFormat("%2i FPS", fps), posX, posY, 20, color);
}

int main(int argc, char *argv[])
{
  int screenWidth = 800;
  int screenHeight = 450;
  int savedX = -1;
  int savedY = -1;

  std::string configPath = "window.state";
  if (FileExists(configPath.c_str()))
  {
    std::cout << "State file found in current directory." << std::endl;
  }
  else
  {
    std::cout << "State file not found. Using defaults." << std::endl;
  }

  std::cout << "Config path: " << configPath << std::endl;

  std::ifstream loadFile(configPath);
  if (loadFile.is_open())
  {
    std::cout << "Loading window state..." << std::endl;
    loadFile >> screenWidth >> screenHeight >> savedX >> savedY;
    loadFile.close();
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, "NEAT Base line");

  if (argc >= 3)
  {
    std::string arg1 = argv[1];
    if (arg1 == "center")
    {
      int monitor = std::atoi(argv[2]);
      if (monitor >= 0 && monitor < GetMonitorCount())
      {
        Vector2 monitorPos = GetMonitorPosition(monitor);
        int x = (int)monitorPos.x + (GetMonitorWidth(monitor) - screenWidth) / 2;
        int y = (int)monitorPos.y + (GetMonitorHeight(monitor) - screenHeight) / 2;
        SetWindowPosition(x, y);
      }
    }
    else
    {
      int x = std::atoi(argv[1]);
      int y = std::atoi(argv[2]);
      SetWindowPosition(x, y);
    }
  }
  else
  {
    if (savedX != -1 && savedY != -1)
    {
      std::cout << "Restoring window position to: " << savedX << ", " << savedY << std::endl;
      SetWindowPosition(savedX, savedY);
    }
  }

  SetTargetFPS(60);

  IOControl::KeyInput keyInput{};

  while (!WindowShouldClose())
  {
    // ===============================================================
    // --- Input Handling ---
    // ===============================================================
    keyInput.scan();
    keyInput.process();

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

    CustomDrawFPS(screenWidth - 80, screenHeight - 20);

    EndDrawing();
  }

  std::ofstream saveFile(configPath);
  if (saveFile.is_open())
  {
    std::cout << "Saving window state..." << std::endl;
    Vector2 pos = GetWindowPosition();
    saveFile << GetScreenWidth() << " " << GetScreenHeight() << " " << (int)pos.x << " " << (int)pos.y;
    saveFile.close();
  }

  std::cout << "Closing window..." << std::endl;
  CloseWindow();

  return 0;
}