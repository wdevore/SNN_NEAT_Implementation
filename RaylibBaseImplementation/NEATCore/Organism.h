#pragma once

#include <memory>
#include <vector>

#include "Genome.h"
#include "Network.h"
#include "Species.h"
#include "Neat.h"

namespace Neat
{
    const int MAX_METADATA = 128;

    /// @brief
    /// ORGANISM CLASS:
    ///   Organisms are Genomes and Networks with fitness information
    ///   i.e. The genotype and phenotype together
    class Organism
    {
    private:
        /* data */
    public:
        double fitness;                   // A measure of fitness for the Organism
        double orig_fitness;              // A fitness measure that won't change during adjustments
        double error;                     // Used just for reporting purposes
        bool winner;                      // Win marker (if needed for a particular task)
        std::shared_ptr<Network> net;     // The Organism's phenotype
        std::shared_ptr<Genome> gnome;    // The Organism's genotype
        std::shared_ptr<Species> species; // The Organism's Species
        double expected_offspring;        // Number of children this Organism may have
        int generation;                   // Tells which generation this Organism is from
        bool eliminate;                   // Marker for destruction of inferior Organisms
        bool champion;                    // Marks the species champ
        int super_champ_offspring;        // Number of reserved offspring for a population leader
        bool pop_champ;                   // Marks the best in population
        bool pop_champ_child;             // Marks the duplicate child of a champion (for tracking purposes)
        double high_fit;                  // DEBUG variable- high fitness of champ
        int time_alive;                   // When playing in real-time allows knowing the maturity of an individual

        // Track its origin- for debugging or analysis- we can tell how the organism was born
        bool mut_struct_baby;
        bool mate_baby;

        // MetaData for the object
        std::string metadata{};
        bool modified;

        Organism(/* args */);
        ~Organism();

        // ================================================
        // Factories
        // ================================================
        static std::shared_ptr<Organism> makeFromGenome(const Neat &neat,
                                                        double fit,
                                                        const std::shared_ptr<Genome> &g,
                                                        int gen, const std::string &md = "");

        static std::shared_ptr<Organism> makeCopy(const Neat &neat, const Organism &org); // Copy Constructor

        // Regenerate the network based on a change in the genotype
        std::shared_ptr<Network> update_phenotype(const Neat &neat);
    };

    // ================================================
    // Sorting Lamdas
    // ================================================
    // This is used for list sorting of Organisms by fitness..highest fitness first
    bool order_orgs(std::shared_ptr<Organism> x, std::shared_ptr<Organism> y);

    bool order_orgs_by_adjusted_fit(std::shared_ptr<Organism> x, std::shared_ptr<Organism> y);

} // namespace Neat
