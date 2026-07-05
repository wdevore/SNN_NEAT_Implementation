#pragma once

#include <memory>

#include "Genome.h"

namespace Neat
{
    // Forward declarations because of circular dependencies.
    class Species;
    class Organism;

    /// @brief
    /// POPULATION CLASS:
    ///   A Population is a group of Organisms
    ///   including their species
    class Population
    {
    private:
        /* data */

    protected:
        // A Population can be spawned off of a single Genome
        // There will be size Genomes added to the Population
        // The Population does not have to be empty to add Genomes
        bool spawn(const Neat &neat, std::shared_ptr<Genome> g, int size);

    public:
        std::vector<std::shared_ptr<Organism>> organisms; // The organisms in the Population

        std::vector<std::shared_ptr<Species>> species; // Species in the Population. Note that the species should comprise all the genomes

        // ******* Member variables used during reproduction *******
        std::vector<std::shared_ptr<Innovation>> innovations; // For holding the genetic innovations of the newest generation
        int cur_node_id;                                      // Current label number available
        double cur_innov_num;

        int last_species; // The highest species number

        // ******* Fitness Statistics *******
        double mean_fitness;
        double variance;
        double standard_deviation;

        int winnergen; // An integer that when above zero tells when the first winner appeared

        // ******* When do we need to delta code? *******
        double highest_fitness;   // Stagnation detector
        int highest_last_changed; // If too high, leads to delta coding

        Population(/* args */);
        ~Population();

        // ================================================
        // Factories
        // ================================================
        // Construct off of a single spawning Genome
        static std::shared_ptr<Population> makeFromGenome(const Neat &neat,
                                                          std::shared_ptr<Genome> g,
                                                          int size);

        // Construct off of a single spawning Genome without mutation
        static std::shared_ptr<Population> makeFromWithoutMutation(const Neat &neat,
                                                                   std::shared_ptr<Genome> g,
                                                                   int size, float power);

        bool clone(const Neat &neat, std::shared_ptr<Genome> g, int size, float power);

        // ================================================
        // Methods
        // ================================================
        // Separate the Organisms into species
        bool speciate(const Neat &neat);

        // Run verify on all Genomes in this Population (Debugging)
        bool verify();

        // Turnover the population to a new generation using fitness
        // The generation argument is the next generation
        bool epoch(const Neat &neat, int generation);

        // *** Real-time methods ***

        // Places the organisms in species in order from best to worst fitness
        bool rank_within_species();
    };

} // namespace Neat
