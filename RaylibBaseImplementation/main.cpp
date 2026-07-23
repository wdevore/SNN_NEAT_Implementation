#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

#include "raylib.h"

#include "KeyInput.h"
#include "Painter.h"

#include "Neat.h"
#include "Simulation.h"
#include "ExperimentXOR.h"

int main(int argc, char *argv[])
{
  int screenWidth = 800;
  int screenHeight = 450;
  int savedX = -1;
  int savedY = -1;
  const int generationsToRun = 100;

  const std::string configPath = "window.state";
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

  try
  {
    Painter::Painter painter{};

    IOControl::KeyInput keyInput{};

    Neat::Neat neat{};
    neat.initialize();

    bool loaded = neat.loadNeatParams("assets/p2test.ne", true);
    // neat.open_log("/home/iposthuman/Documents/raylib.log");
    neat.open_log("/media/RAMDisk/raylib.log");
    neat.log("Raylib log");

    Neat::ExperimentXOR exor{neat};

    Neat::Simulation simulation{exor};
    simulation.initialize(neat, generationsToRun);

    bool simulationRun{false};

    while (!WindowShouldClose())
    {
      // ===============================================================
      // --- Input Handling ---
      // ===============================================================
      keyInput.scan();

      keyInput.process(painter);

      if (keyInput.keyE.isTapped())
      {
        simulation.reset();
      }

      if (keyInput.keyI.isTapped())
      {
        exor.initialize(neat, generationsToRun);
      }

      if (keyInput.keyO.isTapped())
      {
        simulation.showReport(neat);
      }

      // ===============================================================
      // --- Simulation ---
      // ===============================================================
      if (keyInput.keyR.isTapped())
      {
        simulationRun = !simulationRun;
        if (simulationRun)
          std::cout << "Simulation enabled" << std::endl;
        else
          std::cout << "Simulation disabled" << std::endl;
      }

      if (keyInput.keyS.isTapped() && !simulationRun)
      {
        simulation.step(neat);
      }
      else if (simulationRun)
      {
        // Continuously step
        simulation.step(neat);
      }

      // ===============================================================
      // --- Draw ---
      // ===============================================================
      painter.render(screenWidth, screenHeight);
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception occurred" << std::endl;
  }

  std::cout << "========== Exiting Simulation ==========" << std::endl;

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