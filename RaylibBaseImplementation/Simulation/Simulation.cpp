#include <iostream>

#include "Simulation.h"

namespace Neat
{

    Simulation::Simulation(/* args */)
    {
    }

    Simulation::~Simulation()
    {
    }

    void Simulation::reset()
    {
        std::cout << "Resetting simulation..." << std::endl;
        stepCount = 0;
    }

    void Simulation::step(const Neat &neat)
    {
        std::cout << "Processing... " << stepCount << std::endl;
        stepCount++;
    }

} // namespace Neat
