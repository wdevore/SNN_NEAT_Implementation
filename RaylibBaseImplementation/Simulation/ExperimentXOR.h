#pragma once

#include "Experiments.h"

namespace Neat
{

    class ExperimentsXOR : public Experiments
    {
    private:
        /* data */
    public:
        ExperimentsXOR(/* args */);
        ~ExperimentsXOR();

        void step() override;
        void reset() override;
    };
} // namespace Neat