#include <algorithm>
#include <cmath>

#include "Species.h"
#include "Organism.h"

namespace Neat
{
    Species::Species(/* args */)
    {
    }

    Species::~Species()
    {
    }

    std::shared_ptr<Species> Species::makeFromID(int id)
    {
        auto newSpecies = std::make_shared<Species>();

        newSpecies->id = id;
        newSpecies->age = 1;
        newSpecies->ave_fitness = 0.0;
        newSpecies->expected_offspring = 0;
        newSpecies->novel = false;
        newSpecies->age_of_last_improvement = 0;
        newSpecies->max_fitness = 0;
        newSpecies->max_fitness_ever = 0;
        newSpecies->obliterate = false;

        newSpecies->average_est = 0;

        return newSpecies;
    }

    std::shared_ptr<Species> Species::makeFromNovel(int id, bool novel)
    {
        auto newSpecies = std::make_shared<Species>();

        newSpecies->id = id;
        newSpecies->age = 1;
        newSpecies->ave_fitness = 0.0;
        newSpecies->expected_offspring = 0;
        newSpecies->novel = novel;
        newSpecies->age_of_last_improvement = 0;
        newSpecies->max_fitness = 0;
        newSpecies->max_fitness_ever = 0;
        newSpecies->obliterate = false;

        newSpecies->average_est = 0;

        return newSpecies;
    }

    bool Species::add_Organism(std::shared_ptr<Organism> o)
    {
        organisms.push_back(o);
        return true;
    }

    void Species::adjust_fitness(const Neat &neat)
    {
        // std::cout<<"Species "<<id<<" last improved "<<(age-age_of_last_improvement)<<" steps ago when it moved up to "<<max_fitness_ever<<std::endl;

        int age_debt = (age - age_of_last_improvement + 1) - neat.dropoff_age;

        if (age_debt == 0)
            age_debt = 1;

        for (auto &curorg : organisms)
        {
            // Remember the original fitness before it gets modified
            curorg->orig_fitness = curorg->fitness;

            // Make fitness decrease after a stagnation point dropoff_age
            // Added an if to keep species pristine until the dropoff point
            // obliterate is used in competitive coevolution to mark stagnation
            // by obliterating the worst species over a certain age
            if ((age_debt >= 1) || obliterate)
            {
                // Possible graded dropoff
                //(curorg->fitness)=(curorg->fitness)*(-atan(age_debt));

                // Extreme penalty for a long period of stagnation (divide fitness by 100)
                curorg->fitness = curorg->fitness * 0.01;
                // std::cout<<"OBLITERATE Species "<<id<<" of age "<<age<<std::endl;
                // std::cout<<"dropped fitness to "<<(curorg->fitness)<<std::endl;
            }

            // Give a fitness boost up to some young age (niching)
            // The age_significance parameter is a system parameter
            //   if it is 1, then young species get no fitness boost
            if (age <= 10)
                curorg->fitness = curorg->fitness * neat.age_significance;

            // Do not allow negative fitness
            if (curorg->fitness < 0.0)
                curorg->fitness = 0.0001;

            // Share fitness with the species
            curorg->fitness = curorg->fitness / (organisms.size());
        }

        // Sort the population and mark for death those after survival_thresh*pop_size
        // organisms.qsort(order_orgs);
        std::sort(organisms.begin(), organisms.end(), order_orgs);

        // Update age_of_last_improvement here
        if (((*(organisms.begin()))->orig_fitness) > max_fitness_ever)
        {
            age_of_last_improvement = age;
            max_fitness_ever = ((*(organisms.begin()))->orig_fitness);
        }

        // Decide how many get to reproduce based on survival_thresh * pop_size
        // Adding 1.0 ensures that at least one will survive
        int num_parents = (int)std::floor((neat.survival_thresh * ((double)organisms.size())) + 1.0);

        // Mark for death those who are ranked too low to be parents
        if (!organisms.empty())
        {
            organisms.front()->champion = true; // Mark the champ as such
            for (size_t i = num_parents; i < organisms.size(); ++i)
            {
                organisms[i]->eliminate = true; // Mark for elimination
            }
        }
    }

    double Species::compute_average_fitness()
    {
        if (organisms.empty())
        {
            ave_fitness = 0.0;
        }
        else
        {
            double total = 0.0;

            for (const auto &curorg : organisms)
            {
                total += curorg->fitness;
            }
            ave_fitness = total / organisms.size();
        }

        return ave_fitness;
    }

    double Species::compute_max_fitness()
    {
        double max = 0.0;

        for (const auto &curorg : organisms)
        {
            if ((curorg->fitness) > max)
                max = curorg->fitness;
        }

        max_fitness = max;

        return max;
    }

    double Species::count_offspring(double skim)
    {
        int e_o_intpart;     // The floor of an organism's expected offspring
        double e_o_fracpart; // Expected offspring fractional part
        double skim_intpart; // The whole offspring in the skim

        expected_offspring = 0;

        for (const auto &curorg : organisms)
        {
            e_o_intpart = (int)std::floor(curorg->expected_offspring);
            e_o_fracpart = std::fmod(curorg->expected_offspring, 1.0);

            expected_offspring += e_o_intpart;

            // Skim off the fractional offspring
            skim += e_o_fracpart;

            // NOTE:  Some precision is lost by computer
            //        Must be remedied later
            if (skim > 1.0)
            {
                skim_intpart = floor(skim);
                expected_offspring += (int)skim_intpart;
                skim -= skim_intpart;
            }
        }

        return skim;
    }

    bool Species::remove_org(std::shared_ptr<Organism> org)
    {
        auto curorg = std::find(organisms.begin(), organisms.end(), org);

        if (curorg == organisms.end())
        {
            // cout<<"ALERT: Attempt to remove nonexistent Organism from Species"<<endl;
            return false;
        }
        else
        {
            organisms.erase(curorg);
            return true;
        }
    }

    std::shared_ptr<Organism> Species::get_champ()
    {
        double champ_fitness = -1.0;
        std::shared_ptr<Organism> thechamp;

        for (const auto &curorg : organisms)
        {
            // TODO: Remove DEBUG code
            // cout<<"searching for champ...looking at org "<<(*curorg)->gnome->genome_id<<" fitness: "<<(*curorg)->fitness<<endl;
            if (curorg->fitness > champ_fitness)
            {
                thechamp = curorg;
                champ_fitness = thechamp->fitness;
            }
        }

        // cout<<"returning champ #"<<thechamp->gnome->genome_id<<endl;

        return thechamp;
    }

    bool Species::reproduce(
        const Neat &neat,
        int generation,
        Population *pop,
        std::vector<std::shared_ptr<Species>> &sorted_species)
    {
        int count;
        std::vector<std::shared_ptr<Organism>>::iterator curorg;

        int poolsize; // The number of Organisms in the old generation

        int orgnum; // Random variable
        int orgcount;
        std::shared_ptr<Organism> mom; // Parent Organisms
        std::shared_ptr<Organism> dad;
        std::shared_ptr<Organism> baby; // The new Organism

        std::shared_ptr<Genome> new_genome; // For holding baby's genes

        std::vector<std::shared_ptr<Species>>::iterator curspecies; // For adding baby
        std::shared_ptr<Species> newspecies;                        // For babies in new Species
        std::shared_ptr<Organism> comporg;                          // For Species determination through comparison

        std::shared_ptr<Species> randspecies; // For mating outside the Species
        double randmult;
        int randspeciesnum;
        int spcount;
        std::vector<std::shared_ptr<Species>>::iterator cursp;

        std::shared_ptr<Network> net_analogue; // For adding link to test for recurrency
        int pause;

        bool outside;

        bool found; // When a Species is found

        bool champ_done = false; // Flag the preservation of the champion

        std::shared_ptr<Organism> thechamp;

        int giveup; // For giving up finding a mate outside the species

        bool mut_struct_baby;
        bool mate_baby;

        // The weight mutation power is species specific depending on its age
        double mut_power = neat.weight_mut_power;

        // Roulette wheel variables
        double total_fitness = 0.0;
        double marble; // The marble will have a number between 0 and total_fitness
        double spin;   // 0Fitness total while the wheel is spinning

        // Compute total fitness of species for a roulette wheel
        // Note: You don't get much advantage from a roulette here
        //  because the size of a species is relatively small.
        //  But you can use it by using the roulette code here
        // for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
        //   total_fitness+=(*curorg)->fitness;
        // }

        // Check for a mistake
        if ((expected_offspring > 0) &&
            (organisms.size() == 0))
        {
            //    std::cout<<"ERROR:  ATTEMPT TO REPRODUCE OUT OF EMPTY SPECIES"<<std::endl;
            return false;
        }

        poolsize = organisms.size() - 1;

        thechamp = (*(organisms.begin()));

        // Create the designated number of offspring for the Species
        // one at a time
        for (count = 0; count < expected_offspring; count++)
        {

            mut_struct_baby = false;
            mate_baby = false;

            outside = false;

            // Debug Trap
            if (expected_offspring > neat.pop_size)
            {
                //      std::cout<<"ALERT: EXPECTED OFFSPRING = "<<expected_offspring<<std::endl;
                //      cin>>pause;
            }

            // If we have a super_champ (Population champion), finish off some special clones
            if ((thechamp->super_champ_offspring) > 0)
            {
                mom = thechamp;
                new_genome = (mom->gnome)->duplicate(neat, count);

                if ((thechamp->super_champ_offspring) == 1)
                {
                }

                // Most superchamp offspring will have their connection weights mutated only
                // The last offspring will be an exact duplicate of this super_champ
                // Note: Superchamp offspring only occur with stolen babies!
                //       Settings used for published experiments did not use this
                if ((thechamp->super_champ_offspring) > 1)
                {
                    if ((neat.randfloat() < 0.8) ||
                        (neat.mutate_add_link_prob == 0.0))
                        // ABOVE LINE IS FOR:
                        // Make sure no links get added when the system has link adding disabled
                        new_genome->mutate_link_weights(neat, mut_power, 1.0, GAUSSIAN);
                    else
                    {
                        // Sometimes we add a link to a superchamp
                        new_genome->genesis(neat, generation);
                        new_genome->mutate_add_link(neat, (pop->innovations), pop->cur_innov_num, neat.newlink_tries);
                        mut_struct_baby = true;
                    }
                }

                baby = Organism::makeFromGenome(neat, 0.0, new_genome, generation);

                if ((thechamp->super_champ_offspring) == 1)
                {
                    if (thechamp->pop_champ)
                    {
                        // std::cout<<"The new org baby's genome is "<<baby->gnome<<std::endl;
                        baby->pop_champ_child = true;
                        baby->high_fit = mom->orig_fitness;
                    }
                }

                thechamp->super_champ_offspring--;
            }
            // If we have a Species champion, just clone it
            else if ((!champ_done) &&
                     (expected_offspring > 5))
            {

                mom = thechamp; // Mom is the champ

                new_genome = (mom->gnome)->duplicate(neat, count);

                // Baby is just like mommy
                baby = Organism::makeFromGenome(neat, 0.0, new_genome, generation);

                champ_done = true;
            }
            // First, decide whether to mate or mutate
            // If there is only one organism in the pool, then always mutate
            else if ((neat.randfloat() < neat.mutate_only_prob) || poolsize == 0)
            {

                // Choose the random parent

                // RANDOM PARENT CHOOSER
                orgnum = neat.randint(0, poolsize);
                curorg = organisms.begin();
                for (orgcount = 0; orgcount < orgnum; orgcount++)
                    ++curorg;

                ////Roulette Wheel
                // marble=randfloat()*total_fitness;
                // curorg=organisms.begin();
                // spin=(*curorg)->fitness;
                // while(spin<marble) {
                //++curorg;

                ////Keep the wheel spinning
                // spin+=(*curorg)->fitness;
                // }
                ////Finished roulette
                //

                mom = (*curorg);

                new_genome = (mom->gnome)->duplicate(neat, count);

                // Do the mutation depending on probabilities of
                // various mutations

                if (neat.randfloat() < neat.mutate_add_node_prob)
                {
                    // std::cout<<"mutate add node"<<std::endl;
                    new_genome->mutate_add_node(neat, pop->innovations, pop->cur_node_id, pop->cur_innov_num);
                    mut_struct_baby = true;
                }
                else if (neat.randfloat() < neat.mutate_add_link_prob)
                {
                    // std::cout<<"mutate add link"<<std::endl;
                    new_genome->genesis(neat, generation);
                    new_genome->mutate_add_link(neat, (pop->innovations), pop->cur_innov_num, neat.newlink_tries);

                    mut_struct_baby = true;
                }
                // NOTE:  A link CANNOT be added directly after a node was added because the phenotype
                //        will not be appropriately altered to reflect the change
                else
                {
                    // If we didn't do a structural mutation, we do the other kinds

                    if (neat.randfloat() < neat.mutate_random_trait_prob)
                    {
                        // std::cout<<"mutate random trait"<<std::endl;
                        new_genome->mutate_random_trait(neat);
                    }
                    if (neat.randfloat() < neat.mutate_link_trait_prob)
                    {
                        // std::cout<<"mutate_link_trait"<<std::endl;
                        new_genome->mutate_link_trait(neat, 1);
                    }
                    if (neat.randfloat() < neat.mutate_node_trait_prob)
                    {
                        // std::cout<<"mutate_node_trait"<<std::endl;
                        new_genome->mutate_node_trait(neat, 1);
                    }
                    if (neat.randfloat() < neat.mutate_link_weights_prob)
                    {
                        // std::cout<<"mutate_link_weights"<<std::endl;
                        new_genome->mutate_link_weights(neat, mut_power, 1.0, GAUSSIAN);
                    }
                    if (neat.randfloat() < neat.mutate_toggle_enable_prob)
                    {
                        // std::cout<<"mutate toggle enable"<<std::endl;
                        new_genome->mutate_toggle_enable(neat, 1);
                    }
                    if (neat.randfloat() < neat.mutate_gene_reenable_prob)
                    {
                        // std::cout<<"mutate gene reenable"<<std::endl;
                        new_genome->mutate_gene_reenable();
                    }
                }

                baby = Organism::makeFromGenome(neat, 0.0, new_genome, generation);
            }

            // Otherwise we should mate
            else
            {

                // Choose the random mom
                orgnum = neat.randint(0, poolsize);
                curorg = organisms.begin();
                for (orgcount = 0; orgcount < orgnum; orgcount++)
                    ++curorg;

                ////Roulette Wheel
                // marble=randfloat()*total_fitness;
                // curorg=organisms.begin();
                // spin=(*curorg)->fitness;
                // while(spin<marble) {
                //++curorg;

                ////Keep the wheel spinning
                // spin+=(*curorg)->fitness;
                // }
                ////Finished roulette
                //

                mom = (*curorg);

                // Choose random dad

                if ((neat.randfloat() > neat.interspecies_mate_rate))
                {
                    // Mate within Species

                    orgnum = neat.randint(0, poolsize);
                    curorg = organisms.begin();
                    for (orgcount = 0; orgcount < orgnum; orgcount++)
                        ++curorg;

                    ////Use a roulette wheel
                    // marble=randfloat()*total_fitness;
                    // curorg=organisms.begin();
                    // spin=(*curorg)->fitness;
                    // while(spin<marble) {
                    //++curorg;
                    // }

                    ////Keep the wheel spinning
                    // spin+=(*curorg)->fitness;
                    // }
                    ////Finished roulette
                    //

                    dad = (*curorg);
                }
                else
                {
                    // Mate outside Species
                    randspecies = shared_from_this();

                    // Select a random species
                    giveup = 0; // Give up if you cant find a different Species
                    while ((randspecies == shared_from_this()) && (giveup < 5))
                    {
                        // This old way just chose any old species
                        // randspeciesnum=randint(0,(pop->species).size()-1);

                        // Choose a random species tending towards better species
                        randmult = neat.gaussrand() / 4;
                        if (randmult > 1.0)
                            randmult = 1.0;
                        // This tends to select better species
                        randspeciesnum = (int)floor((randmult * (sorted_species.size() - 1.0)) + 0.5);
                        cursp = (sorted_species.begin());
                        for (spcount = 0; spcount < randspeciesnum; spcount++)
                            ++cursp;
                        randspecies = (*cursp);

                        ++giveup;
                    }

                    // OLD WAY: Choose a random dad from the random species
                    // Select a random dad from the random Species
                    // NOTE:  It is possible that a mating could take place
                    //        here between the mom and a baby from the NEW
                    //        generation in some other Species
                    // orgnum=randint(0,(randspecies->organisms).size()-1);
                    // curorg=(randspecies->organisms).begin();
                    // for(orgcount=0;orgcount<orgnum;orgcount++)
                    //   ++curorg;
                    // dad=(*curorg);

                    // New way: Make dad be a champ from the random species
                    dad = (*((randspecies->organisms).begin()));

                    outside = true;
                }

                // Perform mating based on probabilities of differrent mating types
                if (neat.randfloat() < neat.mate_multipoint_prob)
                {
                    new_genome = (mom->gnome)->mate_multipoint(neat, dad->gnome, count, mom->orig_fitness, dad->orig_fitness, outside);
                }
                else if (neat.randfloat() < (neat.mate_multipoint_avg_prob / (neat.mate_multipoint_avg_prob + neat.mate_singlepoint_prob)))
                {
                    new_genome = (mom->gnome)->mate_multipoint_avg(neat, dad->gnome, count, mom->orig_fitness, dad->orig_fitness, outside);
                }
                else
                {
                    new_genome = (mom->gnome)->mate_singlepoint(neat, dad->gnome, count);
                }

                mate_baby = true;

                // Determine whether to mutate the baby's Genome
                // This is done randomly or if the mom and dad are the same organism
                if ((neat.randfloat() > neat.mate_only_prob) ||
                    ((dad->gnome)->genome_id == (mom->gnome)->genome_id) ||
                    (((dad->gnome)->compatibility(neat, mom->gnome)) == 0.0))
                {

                    // Do the mutation depending on probabilities of
                    // various mutations
                    if (neat.randfloat() < neat.mutate_add_node_prob)
                    {
                        new_genome->mutate_add_node(neat, pop->innovations, pop->cur_node_id, pop->cur_innov_num);
                        //  std::cout<<"mutate_add_node: "<<new_genome<<std::endl;
                        mut_struct_baby = true;
                    }
                    else if (neat.randfloat() < neat.mutate_add_link_prob)
                    {
                        new_genome->genesis(neat, generation);
                        new_genome->mutate_add_link(neat, (pop->innovations), pop->cur_innov_num, neat.newlink_tries);

                        // std::cout<<"mutate_add_link: "<<new_genome<<std::endl;
                        mut_struct_baby = true;
                    }
                    else
                    {
                        // Only do other mutations when not doing sturctural mutations

                        if (neat.randfloat() < neat.mutate_random_trait_prob)
                        {
                            new_genome->mutate_random_trait(neat);
                            // std::cout<<"..mutate random trait: "<<new_genome<<std::endl;
                        }
                        if (neat.randfloat() < neat.mutate_link_trait_prob)
                        {
                            new_genome->mutate_link_trait(neat, 1);
                            // std::cout<<"..mutate link trait: "<<new_genome<<std::endl;
                        }
                        if (neat.randfloat() < neat.mutate_node_trait_prob)
                        {
                            new_genome->mutate_node_trait(neat, 1);
                            // std::cout<<"mutate_node_trait: "<<new_genome<<std::endl;
                        }
                        if (neat.randfloat() < neat.mutate_link_weights_prob)
                        {
                            new_genome->mutate_link_weights(neat, mut_power, 1.0, GAUSSIAN);
                            // std::cout<<"mutate_link_weights: "<<new_genome<<std::endl;
                        }
                        if (neat.randfloat() < neat.mutate_toggle_enable_prob)
                        {
                            new_genome->mutate_toggle_enable(neat, 1);
                            // std::cout<<"mutate_toggle_enable: "<<new_genome<<std::endl;
                        }
                        if (neat.randfloat() < neat.mutate_gene_reenable_prob)
                        {
                            new_genome->mutate_gene_reenable();
                            // std::cout<<"mutate_gene_reenable: "<<new_genome<<std::endl;
                        }
                    }

                    // Create the baby
                    baby = Organism::makeFromGenome(neat, 0.0, new_genome, generation);
                }
                else
                {
                    // Create the baby without mutating first
                    baby = Organism::makeFromGenome(neat, 0.0, new_genome, generation);
                }
            }

            // Add the baby to its proper Species
            // If it doesn't fit a Species, create a new one

            baby->mut_struct_baby = mut_struct_baby;
            baby->mate_baby = mate_baby;

            curspecies = (pop->species).begin();
            if (curspecies == (pop->species).end())
            {
                // Create the first species
                newspecies = Species::makeFromNovel(++(pop->last_species), false);
                (pop->species).push_back(newspecies);

                newspecies->add_Organism(baby); // Add the baby
                baby->species = newspecies;     // Point the baby to its species
            }
            else
            {
                comporg = (*curspecies)->first();
                found = false;
                while ((curspecies != (pop->species).end()) &&
                       (!found))
                {
                    if (comporg == 0)
                    {
                        // Keep searching for a matching species
                        ++curspecies;
                        if (curspecies != (pop->species).end())
                            comporg = (*curspecies)->first();
                    }
                    else if (((baby->gnome)->compatibility(neat, comporg->gnome)) < neat.compat_threshold)
                    {
                        // Found compatible species, so add this organism to it
                        (*curspecies)->add_Organism(baby);
                        baby->species = (*curspecies); // Point organism to its species
                        found = true;                  // Note the search is over
                    }
                    else
                    {
                        // Keep searching for a matching species
                        ++curspecies;
                        if (curspecies != (pop->species).end())
                            comporg = (*curspecies)->first();
                    }
                }

                // If we didn't find a match, create a new species
                if (found == false)
                {
                    newspecies = Species::makeFromNovel(++(pop->last_species), true);

                    // std::std::cout<<"CREATING NEW SPECIES "<<pop->last_species<<std::std::endl;
                    (pop->species).push_back(newspecies);
                    newspecies->add_Organism(baby); // Add the baby
                    baby->species = newspecies;     // Point baby to its species
                }

            } // end else
        }

        return true;
    }

    bool Species::rank()
    {
        // organisms.qsort(order_orgs);
        std::sort(organisms.begin(), organisms.end(), order_orgs);
        return true;
    }

    bool order_species(std::shared_ptr<Species> x, std::shared_ptr<Species> y)
    {
        // std::cout<<"Comparing "<<((*((x->organisms).begin()))->orig_fitness)<<" and "<<((*((y->organisms).begin()))->orig_fitness)<<": "<<(((*((x->organisms).begin()))->orig_fitness) > ((*((y->organisms).begin()))->orig_fitness))<<std::endl;
        return (((*((x->organisms).begin()))->orig_fitness) > ((*((y->organisms).begin()))->orig_fitness));
    }

    bool order_new_species(std::shared_ptr<Species> x, std::shared_ptr<Species> y)
    {
        return (x->compute_max_fitness() > y->compute_max_fitness());
    }

} // namespace Neat
