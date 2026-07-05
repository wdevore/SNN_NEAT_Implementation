#pragma once

#include <string>

namespace Neat
{

    class Neat
    {
    private:
        /* data */
    public:
        const int num_trait_params = 8;

        int experiment{0};

        double trait_param_mut_prob{0};
        double trait_mutation_power{0}; // Power of mutation on a signle trait param
        double linktrait_mut_sig{0};    // Amount that mutation_num changes for a trait change inside a link
        double nodetrait_mut_sig{0};    // Amount a mutation_num changes on a link connecting a node that changed its trait
        double weight_mut_power{0};     // The power of a linkweight mutation
        double recur_prob{0};           // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent
        double disjoint_coeff{0};
        double excess_coeff{0};
        double mutdiff_coeff{0};
        double compat_threshold{0};
        double age_significance{0}; // How much does age matter?
        double survival_thresh{0};  // Percent of ave fitness for survival
        double mutate_only_prob{0}; // Prob. of a non-mating reproduction
        double mutate_random_trait_prob{0};
        double mutate_link_trait_prob{0};
        double mutate_node_trait_prob{0};
        double mutate_link_weights_prob{0};
        double mutate_toggle_enable_prob{0};
        double mutate_gene_reenable_prob{0};
        double mutate_add_node_prob{0};
        double mutate_add_link_prob{0};
        double interspecies_mate_rate{0}; // Prob. of a mate being outside species
        double mate_multipoint_prob{0};
        double mate_multipoint_avg_prob{0};
        double mate_singlepoint_prob{0};
        double mate_only_prob{0};  // Prob. of mating without mutation
        double recur_only_prob{0}; // Probability of forcing selection of ONLY links that are naturally recurrent

        int pop_size{0};      // Size of population
        int dropoff_age{0};   // Age where Species starts to be penalized
        int newlink_tries{0}; // Number of tries mutate_add_link will attempt to find an open link
        int print_every{0};   // Tells to print population to file every n generations
        int babies_stolen{0}; // The number of babies to siphen off to the champions
        int num_runs{0};

        Neat(/* args */);
        ~Neat();

        int getUnitCount(const std::string &string, const std::string &set);
        bool loadNeatParams(const std::string &file, bool output);
        double gaussrand() const;

        inline int randposneg() const
        {
            if (rand() % 2)
                return 1;
            else
                return -1;
        }

        inline int randint(int x, int y) const
        {
            return rand() % (y - x + 1) + x;
        }

        inline double randfloat() const
        {
            return rand() / (double)RAND_MAX;
        }

        // SIGMOID FUNCTION ********************************
        // This is a signmoidal activation function, which is an S-shaped squashing function
        // It smoothly limits the amplitude of the output of a neuron to between 0 and 1
        // It is a helper to the neural-activation function get_active_out
        // It is made inline so it can execute quickly since it is at every non-sensor
        // node in a network.
        // NOTE:  In order to make node insertion in the middle of a link possible,
        // the signmoid can be shifted to the right and more steeply sloped:
        // slope=4.924273
        // constant= 2.4621365
        // These parameters optimize mean squared error between the old output,
        // and an output of a node inserted in the middle of a link between
        // the old output and some other node.
        // When not right-shifted, the steepened slope is closest to a linear
        // ascent as possible between -0.5 and 0.5
        double fsigmoid(double activesum, double slope, double constant) const;
        double oldhebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) const;
        double hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) const;
    };

} // namespace Neat
