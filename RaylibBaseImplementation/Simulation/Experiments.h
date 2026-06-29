#pragma once

namespace Neat
{
    class Experiments
    {
    private:
        /* data */
    public:
        Experiments(/* args */);
        ~Experiments();

        virtual void step() = 0;
        virtual void reset() = 0;
    };

} // namespace Neat
