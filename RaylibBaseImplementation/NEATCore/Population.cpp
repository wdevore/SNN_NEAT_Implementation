#include <algorithm>
#include <iostream>

#include "Population.h"
#include "Organism.h"

namespace Neat
{

    Population::Population(/* args */)
    {
    }

    Population::~Population()
    {
    }

    std::shared_ptr<Population> Population::makeFromGenome(const Neat &neat,
                                                           std::shared_ptr<Genome> g,
                                                           int size)
    {
        auto newPop = std::make_shared<Population>();

        newPop->winnergen = 0;
        newPop->highest_fitness = 0.0;
        newPop->highest_last_changed = 0;
        newPop->spawn(neat, g, size);

        return newPop;
    }

    std::shared_ptr<Population> Population::makeFromWithoutMutation(const Neat &neat,
                                                                    std::shared_ptr<Genome> g,
                                                                    int size, float power)
    {
        auto newPop = std::make_shared<Population>();

        newPop->winnergen = 0;
        newPop->highest_fitness = 0.0;
        newPop->highest_last_changed = 0;
        newPop->clone(neat, g, size, power);

        return newPop;
    }

    bool Population::clone(const Neat &neat, std::shared_ptr<Genome> g, int size, float power)
    {
        neat.log("Population::clone START ");
        int count;
        std::shared_ptr<Genome> new_genome;
        std::shared_ptr<Organism> new_organism;

        neat.log("(size, power): ", size, power);

        new_genome = g->duplicate(neat, 1);
        new_organism = Organism::makeFromGenome(neat, 0.0, new_genome, 1);
        organisms.push_back(new_organism);
        neat.log("Added organism to population: ", (int)organisms.size());

        // Create size copies of the Genome
        // Start with perturbed linkweights
        for (count = 2; count <= size; count++)
        {
            neat.log("CREATING ORGANISM count: ", count);
            new_genome = g->duplicate(neat, count);
            if (power > 0)
                new_genome->mutate_link_weights(neat, power, 1.0, GAUSSIAN);

            new_genome->randomize_traits(neat);
            new_organism = Organism::makeFromGenome(neat, 0.0, new_genome, 1);
            organisms.push_back(new_organism);
            neat.log("Rand Trait=>Added organism to population: ", (int)organisms.size());
        }

        if (!organisms.empty())
        {
            // Keep a record of the innovation and node number we are on
            cur_node_id = new_genome->get_last_node_id();
            cur_innov_num = new_genome->get_last_gene_innovnum();
        }

        // Separate the new Population into species
        speciate(neat);

        neat.log("Population::clone END ");

        return true;
    }

    bool Population::spawn(const Neat &neat, std::shared_ptr<Genome> g, int size)
    {
        neat.log("Population::spawn START ");

        std::shared_ptr<Genome> new_genome;
        std::shared_ptr<Organism> new_organism;

        // Create 'size' copies of the Genome
        // Start with perturbed link weights
        for (int id = 1; id <= size; id++)
        {
            neat.log("CREATING ORGANISM count: ", id);

            new_genome = g->duplicate(neat, id);
            // new_genome->mutate_link_weights(1.0,1.0,Mutator::GAUSSIAN);
            new_genome->mutate_link_weights(neat, 1.0, 1.0, Mutator::COLDGAUSSIAN);
            new_genome->randomize_traits(neat);

            new_organism = Organism::makeFromGenome(neat, 0.0, new_genome, 1);
            organisms.push_back(new_organism);
        }

        // Only update innovation numbers if new organisms were actually created
        if (!organisms.empty())
        {
            // Keep a record of the innovation and node number we are on
            auto lastOrganism = organisms.back();
            cur_node_id = lastOrganism->gnome->get_last_node_id();
            cur_innov_num = lastOrganism->gnome->get_last_gene_innovnum();
        }

        // Separate the new Population into species
        speciate(neat);

        neat.log("Population::spawn END ");

        return true;
    }

    bool Population::speciate(const Neat &neat)
    {
        neat.log("Population::speciate START OrgSize: ", (int)organisms.size());

        std::vector<std::shared_ptr<Species>>::iterator curspecies; // Steps through species
        std::shared_ptr<Organism> comporg;                          // Organism for comparison
        std::shared_ptr<Species> newspecies;                        // For adding a new species

        int counter = 0; // Species counter

        // Step through all existing organisms
        for (auto &curorg : organisms)
        {
            // For each organism, search for a species it is compatible to
            curspecies = species.begin();
            if (curspecies == species.end())
            {
                // Create the first species
                newspecies = Species::makeFromID(++counter);
                species.push_back(newspecies);
                newspecies->add_Organism(neat, curorg); // Add the current organism
                neat.log("Adding organism to Species: ", (int)species.size());
                curorg->species = newspecies; // Point organism to its species
            }
            else
            {
                comporg = (*curspecies)->first();
                while (comporg && curspecies != species.end())
                {
                    if (curorg->gnome->compatibility(neat, comporg->gnome) < neat.compat_threshold)
                    {
                        // Found compatible species, so add this organism to it
                        (*curspecies)->add_Organism(neat, curorg);
                        curorg->species = (*curspecies); // Point organism to its species
                        comporg = nullptr;               // Note the search is over
                    }
                    else
                    {
                        // Keep searching for a matching species
                        ++curspecies;
                        if (curspecies != species.end())
                            comporg = (*curspecies)->first();
                    }
                }

                // If we didn't find a match, create a new species
                if (comporg)
                {
                    newspecies = Species::makeFromID(++counter);
                    species.push_back(newspecies);
                    newspecies->add_Organism(neat, curorg); // Add the current organism
                    curorg->species = newspecies;           // Point organism to its species
                }

            } // end else

        } // end for

        last_species = counter; // Keep track of highest species

        neat.log("Population::speciate END ");

        return true;
    }

    bool Population::verify()
    {
        bool verification;

        for (auto &curorg : organisms)
        {
            verification = curorg->gnome->verify();
        }

        return verification;
    }

    bool Population::epoch(const Neat &neat, int generation)
    {
        neat.log("########## Population::epoch START ############");
        std::vector<std::shared_ptr<Species>>::iterator curspecies;
        std::vector<std::shared_ptr<Species>>::iterator deadspecies; // For removing empty Species

        std::vector<std::shared_ptr<Organism>>::iterator curorg;
        std::vector<std::shared_ptr<Organism>>::iterator deadorg;

        std::vector<std::shared_ptr<Innovation>>::iterator curinnov;
        std::vector<std::shared_ptr<Innovation>>::iterator deadinnov; // For removing old Innovs

        double total{0.0}; // Used to compute average fitness over all Organisms

        double overall_average{0.0}; // The average modified fitness among ALL organisms

        int orgcount{0};

        // The fractional parts of expected offspring that can be
        // Used only when they accumulate above 1 for the purposes of counting
        // Offspring
        double skim{0.0};
        int total_expected{0}; // precision checking
        int total_organisms = organisms.size();
        int max_expected{0};
        std::shared_ptr<Species> best_species;
        int final_expected{0};

        int pause;

        // Rights to make babies can be stolen from inferior species
        // and given to their superiors, in order to concentrate exploration on
        // the best species
        int NUM_STOLEN = neat.babies_stolen; // Number of babies to steal
        int one_fifth_stolen{0};
        int one_tenth_stolen{0};

        std::vector<std::shared_ptr<Species>> sorted_species; // Species sorted by max fit org in Species
        int stolen_babies{0};                                 // Babies taken from the bad species and given to the champs

        int half_pop{0};

        int best_species_num{0}; // Used in debugging to see why (if) best species dies
        bool best_ok{false};

        // We can try to keep the number of species constant at this number
        int num_species_target = 4;
        int num_species = species.size();
        // double compat_mod = 0.3; // Modify compat thresh to control speciation

        // Keeping species diverse
        // This commented out code forces the system to aim for
        //  num_species species at all times, enforcing diversity
        // This tinkers with the compatibility threshold, which
        //  normally would be held constant
        /*
        if (generation>1) {
            if (num_species<num_species_target)
                neat.compat_threshold-=compat_mod;
            else if (num_species>num_species_target)
                neat.compat_threshold+=compat_mod;

            if (neat.compat_threshold<0.3) neat.compat_threshold=0.3;

        }
        */

        // Stick the Species pointers into a new Species list for sorting
        neat.log("Species in population: ", (int)(species.size()));

        for (const auto &s : species)
        {
            sorted_species.push_back(s);
        }

        // Sort the Species by max fitness (Use an extra list to do this)
        // These need to use ORIGINAL fitness
        // sorted_species.qsort(order_species);
        std::sort(sorted_species.begin(), sorted_species.end(), order_species);

        // Flag the lowest performing species over age 20 every 30 generations
        // NOTE: THIS IS FOR COMPETITIVE COEVOLUTION STAGNATION DETECTION

        // Find the last species that is not young
        auto it = std::find_if(sorted_species.rbegin(), sorted_species.rend(), [](const auto &s)
                               { return s->age >= 20; });
        // neat.log("Found species: ", (*it)->id);

        // If a suitable species is found, get the forward iterator to it.
        // Otherwise, curspecies will be sorted_species.begin().
        // If a species is found, it.base() gives us a forward iterator to the element after the found one.
        // We use std::prev(it.base()) to get the correct iterator to the element itself.
        // If no such species is found, the reverse iterator will be sorted_species.rend(), and it.base() correctly
        // points to sorted_species.begin(), matching the behavior of the original loop.
        curspecies = (it == sorted_species.rend()) ? sorted_species.begin() : std::prev(it.base());

        neat.log("Found curspecies: ", (*curspecies)->id);

        if ((generation % 30) == 0)
            (*curspecies)->obliterate = true;

        neat.log("Number of Species: ", num_species);

        // Use Species' ages to modify the objective fitness of organisms
        //  in other words, make it more fair for younger species
        //  so they have a chance to take hold
        // Also penalize stagnant species
        // Then adjust the fitness using the species size to "share" fitness
        // within a species.
        // Then, within each Species, mark for death
        // those below survival_thresh*average
        for (auto &s : species)
        {
            s->adjust_fitness(neat);
        }

        // Calculate the total fitness of the population and then to assign the
        // expected number of offspring to each organism.

        // Go through the organisms and add up their fitnesses to compute the
        // overall average
        for (const auto &org : organisms)
        {
            total += org->fitness;
            neat.log("total ", total);
            neat.log("org fitness ", org->fitness);
        }
        overall_average = total / total_organisms;
        neat.log("total ", total);
        neat.log("total_organisms ", total_organisms);
        neat.log("(Generation, overall_average): ", generation, overall_average);

        // Now compute expected number of offspring for each individual organism
        for (auto &org : organisms)
        {
            org->expected_offspring = (org->fitness / overall_average);
        }

        // Now add those offspring up within each Species to get the number of
        // offspring per Species
        skim = 0.0;
        total_expected = 0;
        for (auto &s : species)
        {
            skim = s->count_offspring(skim);
            neat.log("########### Population::epoch::Species::count_offspring ############, skim:", skim);

            total_expected += s->expected_offspring;
        }

        // Need to make up for lost foating point precision in offspring assignment
        // If we lost precision, give an extra baby to the best Species
        if (total_expected < total_organisms)
        {
            // Find the Species expecting the most
            max_expected = 0;
            final_expected = 0;
            for (const auto &s : species)
            {
                if (s->expected_offspring >= max_expected)
                {
                    max_expected = s->expected_offspring;
                    best_species = s;
                }
                final_expected += s->expected_offspring;
            }

            // Give the extra offspring to the best species
            if (best_species)
                ++(best_species->expected_offspring);
            else
                neat.log("WARNING: Found no Best Species");

            final_expected++;

            // If we still arent at total, there is a problem
            // Note that this can happen if a stagnant Species
            // dominates the population and then gets killed off by its age
            // Then the whole population plummets in fitness
            // If the average fitness is allowed to hit 0, then we no longer have
            // an average we can use to assign offspring.
            if (final_expected < total_organisms)
            {
                neat.log("######## EPOCH Population died! #########");
                // cin>>pause;
                for (auto &s : species)
                {
                    s->expected_offspring = 0;
                }
                if (best_species)
                    best_species->expected_offspring = total_organisms;
            }
        }

        // Sort the Species by max fitness (Use an extra list to do this)
        // These need to use ORIGINAL fitness
        // sorted_species.qsort(order_species);
        std::sort(sorted_species.begin(), sorted_species.end(), order_species);

        best_species_num = (*(sorted_species.begin()))->id;

        for (const auto &s : sorted_species)
        {
            // Print out for Debugging/viewing what's going on
            neat.log("(Species, Organisms): ", s->id, (int)s->organisms.size());
            neat.log("(Fitness, last improved): ", (*(s->organisms).begin())->orig_fitness, (s->age - s->age_of_last_improvement));
            neat.log("(Age, Avg Fitness): ", s->age, s->ave_fitness);
        }

        // Check for Population-level stagnation
        curspecies = sorted_species.begin();
        (*(((*curspecies)->organisms).begin()))->pop_champ = true; // DEBUG marker of the best of pop
        if (((*(((*curspecies)->organisms).begin()))->orig_fitness) > highest_fitness)
        {
            highest_fitness = ((*(((*curspecies)->organisms).begin()))->orig_fitness);
            highest_last_changed = 0;
            neat.log("NEW POPULATION RECORD FITNESS: ", highest_fitness);
        }
        else
        {
            ++highest_last_changed;
            neat.log("generations since last population fitness record (highest_last_changed, highest_fitness): ", highest_last_changed, highest_fitness);
        }

        // Check for stagnation- if there is stagnation, perform delta-coding
        if (highest_last_changed >= neat.dropoff_age + 5)
        {
            //    cout<<"PERFORMING DELTA CODING"<<endl;

            highest_last_changed = 0;

            half_pop = neat.pop_size / 2;

            //    cout<<"half_pop"<<half_pop<<" pop_size-halfpop: "<<pop_size-half_pop<<endl;

            curspecies = sorted_species.begin();

            (*(((*curspecies)->organisms).begin()))->super_champ_offspring = half_pop;
            (*curspecies)->expected_offspring = half_pop;
            (*curspecies)->age_of_last_improvement = (*curspecies)->age;

            ++curspecies;

            if (curspecies != sorted_species.end())
            {

                (*(((*curspecies)->organisms).begin()))->super_champ_offspring = neat.pop_size - half_pop;
                (*curspecies)->expected_offspring = neat.pop_size - half_pop;
                (*curspecies)->age_of_last_improvement = (*curspecies)->age;

                ++curspecies;

                // Get rid of all species under the first 2
                while (curspecies != sorted_species.end())
                {
                    (*curspecies)->expected_offspring = 0;
                    ++curspecies;
                }
            }
            else
            {
                curspecies = sorted_species.begin();
                (*(((*curspecies)->organisms).begin()))->super_champ_offspring += neat.pop_size - half_pop;
                (*curspecies)->expected_offspring = neat.pop_size - half_pop;
            }
        }
        // STOLEN BABIES:  The system can take expected offspring away from
        //   worse species and give them to superior species depending on
        //   the system parameter babies_stolen (when babies_stolen > 0)
        else if (neat.babies_stolen > 0)
        {
            // Take away a constant number of expected offspring from the worst few species

            stolen_babies = 0;
            curspecies = sorted_species.end();
            curspecies--;
            while ((stolen_babies < NUM_STOLEN) && (curspecies != sorted_species.begin()))
            {
                neat.log("Considering Species (id, age): ", (*curspecies)->id, (*curspecies)->age);
                neat.log("Expected offspring ", (*curspecies)->expected_offspring);

                if ((*curspecies)->age > 5 && (*curspecies)->expected_offspring > 2)
                {
                    neat.log("STEALING!");

                    // This species has enough to finish off the stolen pool
                    if (((*curspecies)->expected_offspring - 1) >= (NUM_STOLEN - stolen_babies))
                    {
                        (*curspecies)->expected_offspring -= (NUM_STOLEN - stolen_babies);
                        stolen_babies = NUM_STOLEN;
                    }
                    // Not enough here to complete the pool of stolen
                    else
                    {
                        stolen_babies += (*curspecies)->expected_offspring - 1;
                        (*curspecies)->expected_offspring = 1;
                    }
                }

                curspecies--;

                if (stolen_babies > 0)
                    neat.log("stolen babies so far: ", stolen_babies);
            }

            // cout<<"STOLEN BABIES: "<<stolen_babies<<endl;

            // Mark the best champions of the top species to be the super champs
            // who will take on the extra offspring for cloning or mutant cloning
            curspecies = sorted_species.begin();

            // Determine the exact number that will be given to the top three
            // They get , in order, 1/5 1/5 and 1/10 of the stolen babies
            one_fifth_stolen = neat.babies_stolen / 5;
            one_tenth_stolen = neat.babies_stolen / 10;

            // Don't give to dying species even if they are champs
            curspecies = std::find_if(curspecies, sorted_species.end(), [&](const auto &s)
                                      { return s->last_improved() <= neat.dropoff_age; });

            // Concentrate A LOT on the number one species
            if ((stolen_babies >= one_fifth_stolen) && (curspecies != sorted_species.end()))
            {
                (*(((*curspecies)->organisms).begin()))->super_champ_offspring = one_fifth_stolen;
                (*curspecies)->expected_offspring += one_fifth_stolen;
                stolen_babies -= one_fifth_stolen;
                neat.log("Gave: (one_fifth_stolen, id): ", one_fifth_stolen, (*curspecies)->id);
                neat.log("The best superchamp is: ", (*(((*curspecies)->organisms).begin()))->gnome->genome_id);

                // Print this champ to file "champ" for observation if desired
                // IMPORTANT:  This causes generational file output
                // print_Genome_tofile((*(((*curspecies)->organisms).begin()))->gnome,"champ");

                curspecies++;
            }

            // Don't give to dying species even if they are champs
            curspecies = std::find_if(curspecies, sorted_species.end(), [&](const auto &s)
                                      { return s->last_improved() <= neat.dropoff_age; });

            if ((curspecies != sorted_species.end()))
            {
                if (stolen_babies >= one_fifth_stolen)
                {
                    (*(((*curspecies)->organisms).begin()))->super_champ_offspring = one_fifth_stolen;
                    (*curspecies)->expected_offspring += one_fifth_stolen;
                    stolen_babies -= one_fifth_stolen;
                    neat.log("Gave: (stolen_babies, id): ", stolen_babies, (*curspecies)->id);
                    curspecies++;
                }
            }

            // Don't give to dying species even if they are champs
            curspecies = std::find_if(curspecies, sorted_species.end(), [&](const auto &s)
                                      { return s->last_improved() <= neat.dropoff_age; });

            if (curspecies != sorted_species.end())
                if (stolen_babies >= one_tenth_stolen)
                {
                    (*(((*curspecies)->organisms).begin()))->super_champ_offspring = one_tenth_stolen;
                    (*curspecies)->expected_offspring += one_tenth_stolen;
                    stolen_babies -= one_tenth_stolen;

                    neat.log("Gave: (one_tenth_stolen, id): ", one_tenth_stolen, (*curspecies)->id);
                    curspecies++;
                }

            // Don't give to dying species even if they are champs
            curspecies = std::find_if(curspecies, sorted_species.end(), [&](const auto &s)
                                      { return s->last_improved() <= neat.dropoff_age; });

            while ((stolen_babies > 0) && (curspecies != sorted_species.end()))
            {
                // Randomize a little which species get boosted by a super champ

                if (neat.randfloat() > 0.1)
                    if (stolen_babies > 3)
                    {
                        (*(((*curspecies)->organisms).begin()))->super_champ_offspring = 3;
                        (*curspecies)->expected_offspring += 3;
                        stolen_babies -= 3;
                        neat.log("Gave 3 babies to Species: ", (*curspecies)->id);
                    }
                    else
                    {
                        // cout<<"3 or less babies available"<<endl;
                        (*(((*curspecies)->organisms).begin()))->super_champ_offspring = stolen_babies;
                        (*curspecies)->expected_offspring += stolen_babies;
                        neat.log("Gave: (babies to Species, id): ", stolen_babies, (*curspecies)->id);
                        stolen_babies = 0;
                    }

                curspecies++;

                // Don't give to dying species even if they are champs
                curspecies = std::find_if(curspecies, sorted_species.end(), [&](const auto &s)
                                          { return s->last_improved() <= neat.dropoff_age; });
            }

            // cout<<"Done giving back babies"<<endl;

            // If any stolen babies aren't taken, give them to species #1's champ
            if (stolen_babies > 0)
            {
                neat.log("Not all given back, giving to best Species");

                curspecies = sorted_species.begin();
                (*(((*curspecies)->organisms).begin()))->super_champ_offspring += stolen_babies;
                (*curspecies)->expected_offspring += stolen_babies;
                stolen_babies = 0;
            }
        }

        // Kill off all Organisms marked for death.  The remainder
        // will be allowed to reproduce.
        curorg = organisms.begin();
        while (curorg != organisms.end())
        {
            neat.log("curorg eliminate: ", (*curorg)->eliminate, true);
            if ((*curorg)->eliminate)
            {
                // Remove the organism from its Species
                neat.log("Remove organism from its Species: ", (*curorg)->species->id);
                (*curorg)->species->remove_org(*curorg);

                // TODO may need to set curorg = nullptr
                // Remember where we are
                deadorg = curorg;
                ++curorg;

                // iter2 =  v.erase(iter);

                // Remove the organism from the master list
                curorg = organisms.erase(deadorg);
            }
            else
            {
                ++curorg;
            }
        }

        neat.log("Performing reproduction");

        // Perform reproduction.  Reproduction is done on a per-Species
        // basis.  (So this could be paralellized potentially.)
        //	for(curspecies=species.begin();curspecies!=species.end();++curspecies) {

        // KENHACK
        //		for(std::vector<Species*>::iterator curspecies2=species.begin();curspecies2!=species.end();++curspecies2) {
        //		  std::cout<<"PRE in repro specloop SPEC EXISTING number "<<(*curspecies2)->id<<std::endl;
        //	}

        //	(*curspecies)->reproduce(generation,this,sorted_species);

        //}

        curspecies = species.begin();
        int last_id = (*curspecies)->id;
        while (curspecies != species.end())
        {
            (*curspecies)->reproduce(neat, generation, this, sorted_species);

            // Set the current species to the id of the last species checked
            //(the iterator must be reset because there were possibly vector insertions during reproduce)
            std::vector<std::shared_ptr<Species>>::iterator curspecies2 = species.begin();
            while (curspecies2 != species.end())
            {
                if (((*curspecies2)->id) == last_id)
                    curspecies = curspecies2;
                curspecies2++;
            }

            // Move to the next on the list
            curspecies++;

            // Record where we are
            if (curspecies != species.end())
                last_id = (*curspecies)->id;
        }

        neat.log("Reproduction Complete");

        // Destroy and remove the old generation from the organisms and species
        curorg = organisms.begin();
        while (curorg != organisms.end())
        {
            auto organ = *curorg;

            // std::cout << "Removing org # " << organ->gnome->genome_id << std::endl;

            // TODO may need to curorg = nullptr

            // Remove the organism from its Species
            if (organ->species)
                organ->species->remove_org(organ);
            // else
            //     std::cout << "No Species for Organism" << std::endl;

            // Remember where we are
            deadorg = curorg;
            ++curorg;
            organ = *curorg;

            // if (organ)
            //     std::cout << "Next org # " << organ->gnome->genome_id << std::endl;
            // else
            //     std::cout << "No Next Organism" << std::endl;

            // Remove the organism from the master list
            curorg = organisms.erase(deadorg);
            organ = *curorg;

            // if (organ)
            //     std::cout << "NNext Organism from master # " << organ->gnome->genome_id << std::endl;
        }

        // Remove all empty Species and age ones that survive
        // As this happens, create master organism list for the new generation
        curspecies = species.begin();
        orgcount = 0;
        while (curspecies != species.end())
        {
            auto currentSpecies = *curspecies;

            if (currentSpecies->organisms.empty())
            {
                deadspecies = curspecies;
                ++curspecies;

                curspecies = species.erase(deadspecies);
            }
            // Age surviving Species and
            // Rebuild master Organism list: NUMBER THEM as they are added to the list
            else
            {
                // Age any Species that is not newly created in this generation
                if (currentSpecies->novel)
                {
                    currentSpecies->novel = false;
                }
                else
                    ++(currentSpecies->age);

                // Go through the organisms of the curspecies and add them to
                // the master list
                for (const auto &org : currentSpecies->organisms)
                {
                    org->gnome->genome_id = orgcount++;
                    organisms.push_back(org);
                }
                ++curspecies;
            }
        }

        // Remove the innovations of the current generation
        curinnov = innovations.begin();
        while (curinnov != innovations.end())
        {
            deadinnov = curinnov;
            ++curinnov;

            curinnov = innovations.erase(deadinnov);
        }

        // DEBUG: Check to see if the best species died somehow
        //  We don't want this to happen
        curspecies = species.begin();
        best_ok = false;
        while (curspecies != species.end())
        {
            if ((*curspecies)->id == best_species_num)
                best_ok = true;
            ++curspecies;
        }

        if (!best_ok)
            neat.log("ERROR: THE BEST SPECIES DIED!");
        else
            neat.log("The best survived: ", best_species_num);

        // DEBUG: Checking the top organism's duplicate in the next gen
        // This prints the champ's child to the screen
        for (const auto &org : organisms)
        {
            if (org->pop_champ_child)
            {
                neat.log("At end of reproduction cycle, the child of the pop champ is: ", org->gnome->genome_id);
            }
        }

        if (stolen_babies > 0)
            neat.log("stolen_babies at end: ", stolen_babies);

        neat.log("########## Population::epoch END ############");

        return true;
    }

    bool Population::rank_within_species()
    {
        // Add each Species in this generation to the snapshot
        for (auto &s : species)
        {
            s->rank();
        }

        return true;
    }

} // namespace Neat
