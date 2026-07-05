#include <iostream>
#include <fstream>

#include "Trait.h"

namespace Neat
{

    Trait::Trait()
    {
    }

    Trait::~Trait()
    {
    }

    Trait::Trait(const Trait &t)
    {
        trait_id = t.trait_id;
        params = t.params;
    }

    std::shared_ptr<Trait> Trait::makeFromParams(const Neat &neat,
                                                 int id,
                                                 double p1, double p2, double p3, double p4,
                                                 double p5, double p6, double p7, double p8, double p9)
    {
        auto newTrait = std::make_shared<Trait>();

        newTrait->trait_id = id;
        newTrait->params[0] = p1;
        newTrait->params[1] = p2;
        newTrait->params[2] = p3;
        newTrait->params[3] = p4;
        newTrait->params[4] = p5;
        newTrait->params[5] = p6;
        newTrait->params[6] = p7;
        newTrait->params[7] = 0;

        newTrait->params.resize(neat.num_trait_params);

        return newTrait;
    }

    std::shared_ptr<Trait> Trait::makeCopy(const Trait &trait)
    {
        return std::make_shared<Trait>(trait);
    }

    std::shared_ptr<Trait> Trait::makeByAverage(const Neat &neat, const Trait &t1, const Trait &t2)
    {
        auto newTrait = std::make_shared<Trait>();

        newTrait->params.clear();
        newTrait->params.resize(t1.params.size());
        newTrait->trait_id = t1.trait_id;

        for (int count = 0; count < t1.params.size(); count++)
            newTrait->params[count] = ((t1.params[count]) + (t2.params)[count]) / 2.0;

        return newTrait;
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
