#include "Organism.h"

namespace Neat
{
    Organism::Organism(/* args */)
    {
        metadata.resize(MAX_METADATA);
    }

    Organism::~Organism()
    {
    }

    std::shared_ptr<Organism> Organism::makeFromGenome(const Neat &neat,
                                                       double fit,
                                                       const std::shared_ptr<Genome> &g,
                                                       int gen, const std::string &md)
    {
        auto newOrg = std::make_shared<Organism>();

        newOrg->fitness = fit;
        newOrg->orig_fitness = fit;
        newOrg->gnome = g;
        newOrg->net = newOrg->update_phenotype(neat);
        // newOrg->species; // Start it in no Species. Note shared pointers default to nullptr
        newOrg->expected_offspring = 0;
        newOrg->generation = gen;
        newOrg->eliminate = false;
        newOrg->error = 0;
        newOrg->winner = false;
        newOrg->champion = false;
        newOrg->super_champ_offspring = 0;

        // If md is null, then we don't have metadata, otherwise we do have metadata so copy it over
        newOrg->metadata = md;

        newOrg->time_alive = 0;

        // DEBUG vars
        newOrg->pop_champ = false;
        newOrg->pop_champ_child = false;
        newOrg->high_fit = 0;
        newOrg->mut_struct_baby = 0;
        newOrg->mate_baby = 0;

        return newOrg;
    }

    std::shared_ptr<Organism> Organism::makeCopy(const Neat &neat, const Organism &org)
    {
        auto newOrg = std::make_shared<Organism>();

        newOrg->fitness = org.fitness;
        newOrg->orig_fitness = org.orig_fitness;
        newOrg->gnome = Genome::makeCopy(neat, *(org.gnome)); // Associative relationship
        newOrg->net = Network::makeCopy(*(org.net));          // Associative relationship
        newOrg->species = org.species;                        // Delegation relationship
        newOrg->expected_offspring = org.expected_offspring;
        newOrg->generation = org.generation;
        newOrg->eliminate = org.eliminate;
        newOrg->error = org.error;
        newOrg->winner = org.winner;
        newOrg->champion = org.champion;
        newOrg->super_champ_offspring = org.super_champ_offspring;

        newOrg->metadata = org.metadata;

        newOrg->time_alive = org.time_alive;
        newOrg->pop_champ = org.pop_champ;
        newOrg->pop_champ_child = org.pop_champ_child;
        newOrg->high_fit = org.high_fit;
        newOrg->mut_struct_baby = org.mut_struct_baby;
        newOrg->mate_baby = org.mate_baby;

        newOrg->modified = false;

        return newOrg;
    }

    std::shared_ptr<Network> Organism::update_phenotype(const Neat &neat)
    {
        // Recreate the phenotype off the new genotype.
        // The old phenotype is automatically deallocated by the shared_ptr's assignment operator.
        auto net = gnome->genesis(neat, gnome->genome_id);

        modified = true;

        return net;
    }

    bool order_orgs(std::shared_ptr<Organism> x, std::shared_ptr<Organism> y)
    {
        return x->fitness > y->fitness;
    }

    bool order_orgs_by_adjusted_fit(std::shared_ptr<Organism> x, std::shared_ptr<Organism> y)
    {
        return x->fitness / x->species->organisms.size() > y->fitness / y->species->organisms.size();
    }

} // namespace Neat
