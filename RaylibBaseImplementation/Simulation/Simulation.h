#pragma once

#include "Neat.h"

namespace Neat
{
    class Simulation
    {
    private:
        /* data */
    public:
        int stepCount{0};

        Simulation(/* args */);
        ~Simulation();

        void reset();
        void step(const Neat &neat);
    };

} // namespace Neat
