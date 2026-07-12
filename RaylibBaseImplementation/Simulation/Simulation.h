#pragma once

#include "Neat.h"
#include "Experiment.h"

namespace Neat
{
    class Simulation
    {
    private:
        /* data */
    public:
        int genId{1};

        Experiment &experiment;

        Simulation(Experiment &experiment) : experiment{experiment} {};
        ~Simulation();

        void reset();
        void initialize(const Neat &neat, int gens);
        void step(const Neat &neat);

        void showReport(const Neat &neat);
    };

} // namespace Neat
