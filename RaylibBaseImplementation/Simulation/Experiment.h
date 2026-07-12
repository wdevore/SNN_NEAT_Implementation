#pragma once
#include <iostream>

#include "Population.h"
#include "Neat.h"

// Each experiment runs for a fixed number of generations.
// Each generation runs until a winner is found.

namespace Neat
{
    class Experiment
    {
    private:
        /* data */
    public:
        int experimentCnt{0}; // How many experiments are run.
        int generationCnt{0}; // How may generations are run for each experiment.

        Experiment();
        Experiment(const Neat &neat);
        ~Experiment();

        virtual void initialize(const Neat &neat, int gens) = 0;

        virtual bool runStep(const Neat &neat, int gen) = 0;
        // Run through a series of generations and stops at the first winner.
        // We can still pause the run.
        virtual bool run(const Neat &neat, bool paused) = 0;
        virtual void post_test(const Neat &neat) = 0;

        virtual void reset() = 0;
    };

} // namespace Neat
