#include <iostream>

#include "Simulation.h"

namespace Neat
{

    Simulation::~Simulation()
    {
    }

    void Simulation::reset()
    {
        std::cout << "Resetting simulation..." << std::endl;
    }

    void Simulation::initialize(const Neat &neat, int gens)
    {
        experiment.initialize(neat, gens);
    }

    void Simulation::step(const Neat &neat)
    {
        std::cout << "=======================================================" << std::endl;
        std::cout << "Stepping generation (" << genId << ")" << std::endl;

        // Make a single step of the experiment.
        bool winnerFound = experiment.runStep(neat, genId);
        if (winnerFound)
        {
            std::cout << "######### Winner found #########" << std::endl;
        }

        genId++;
    }

    void Simulation::showReport(const Neat &neat)
    {
        experiment.post_test(neat);
    }

} // namespace Neat
