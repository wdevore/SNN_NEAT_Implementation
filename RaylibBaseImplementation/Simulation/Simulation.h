#pragma once

#include "Neat.h"

namespace Neat
{
    class Simulation
    {
    private:
        /* data */
    public:
        Simulation(/* args */);
        ~Simulation();

        void process(const Neat &neat);
    };

} // namespace Neat
