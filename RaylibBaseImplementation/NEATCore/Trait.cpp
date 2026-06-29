#include <iostream>
#include <fstream>

#include "Trait.h"

namespace Neat
{

    Trait::Trait(const Neat &neat)
    {
        params.resize(neat.num_trait_params);
    }

    Trait::Trait(const Neat &neat, int id,
                 double p1, double p2, double p3, double p4, double p5,
                 double p6, double p7, double p8, double p9) : Trait(neat)
    {
        trait_id = id;
        params[0] = p1;
        params[1] = p2;
        params[2] = p3;
        params[3] = p4;
        params[4] = p5;
        params[5] = p6;
        params[6] = p7;
        params[7] = 0;
    }

    Trait::Trait(const Trait &t) : trait_id(t.trait_id), params(t.params)
    {
    }

    Trait::Trait(const Trait &t1, const Trait &t2) : trait_id(t1.trait_id)
    {
        params.clear();
        params.resize(t1.params.size());

        for (int count = 0; count < t1.params.size(); count++)
            params[count] = (((t1.params)[count]) + ((t2.params)[count])) / 2.0;
    }

    Trait::~Trait()
    {
    }

    void Trait::toFile(std::ofstream &outFile)
    {
        outFile << "trait " << trait_id << " ";
        for (const auto &parm : params)
            outFile << parm << " ";

        outFile << std::endl;
    }

    void Trait::mutate(const Neat &neat)
    {
        for (auto &parm : params)
        {
            if (neat.randfloat() > neat.trait_param_mut_prob)
            {
                parm += (neat.randposneg() * neat.randfloat()) * neat.trait_mutation_power;
                if (parm < 0)
                    parm = 0;
                if (parm > 1.0)
                    parm = 1.0;
            }
        }
    }
} // namespace Neat
