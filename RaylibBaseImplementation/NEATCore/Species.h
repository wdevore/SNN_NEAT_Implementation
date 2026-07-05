#pragma once

#include <vector>
#include <memory>

#include "Neat.h"
#include "Population.h"

namespace Neat
{
    // Forward declarations for circular dependencies
    class Organism;

    class Species : public std::enable_shared_from_this<Species>
    {
    private:
        /* data */
    public:
        int id;
        int age;                 // The age of the Species
        double ave_fitness;      // The average fitness of the Species
        double max_fitness;      // Max fitness of the Species
        double max_fitness_ever; // The max it ever had
        int expected_offspring;
        bool novel;
        bool checked;
        bool obliterate;                                  // Allows killing off in competitive coevolution stagnation
        std::vector<std::shared_ptr<Organism>> organisms; // The organisms in the Species
        // std::vector<Organism*> reproduction_pool;  //The organisms for reproduction- NOT NEEDED
        int age_of_last_improvement; // If this is too long ago, the Species will goes extinct
        double average_est;          // When playing real-time allows estimating average fitness

        Species(/* args */);
        ~Species();

        // ================================================
        // Factories
        // ================================================
        static std::shared_ptr<Species> makeFromID(int id);

        // Allows the creation of a Species that won't age (a novel one)
        // This protects new Species from aging inside their first generation
        static std::shared_ptr<Species> makeFromNovel(int id, bool novel);

        // ================================================
        // Methods
        // ================================================
        bool add_Organism(std::shared_ptr<Organism> o);

        std::shared_ptr<Organism> first() { return organisms.front(); }

        // Change the fitness of all the organisms in the species to possibly depend slightly on the age of the species
        // and then divide it by the size of the species so that the organisms in the species "share" the fitness
        void adjust_fitness(const Neat &neat);

        double compute_average_fitness();

        double compute_max_fitness();

        // Counts the number of offspring expected from all its members skim is for keeping track of remaining
        //  fractional parts of offspring and distributing them among species
        double count_offspring(double skim);

        // Compute generations since last improvement
        int last_improved() { return age - age_of_last_improvement; }

        // Remove an organism from Species
        bool remove_org(std::shared_ptr<Organism> org);

        double size() { return organisms.size(); }

        std::shared_ptr<Organism> get_champ();

        // Perform mating and mutation to form next generation
        bool reproduce(const Neat &neat,
                       int generation,
                       Population *pop,
                       std::vector<std::shared_ptr<Species>> &sorted_species);

        // Place organisms in this species in order by their fitness
        bool rank();
    };

    // ================================================
    // Sorting Lambdas
    // ================================================
    // This is used for list sorting of Species by fitness of best organism highest fitness first
    bool order_species(std::shared_ptr<Species> x, std::shared_ptr<Species> y);

    bool order_new_species(std::shared_ptr<Species> x, std::shared_ptr<Species> y);

} // namespace Neat
