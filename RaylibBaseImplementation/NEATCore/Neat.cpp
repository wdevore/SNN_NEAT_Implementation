#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>

#include "Neat.h"

namespace Neat
{

    Neat::Neat(/* args */)
    {
    }

    Neat::~Neat()
    {
    }

    /// @brief To test use: getUnit(filestring, delimiters);
    /// @param string
    /// @param set
    /// @return
    int Neat::getUnitCount(const std::string &string, const std::string &set)
    {
        if (string.empty())
        {
            return 0;
        }

        int count = 0;
        bool in_unit = false;
        for (char c : string)
        {
            if (set.find(c) != std::string::npos)
            {
                if (in_unit)
                {
                    count++;
                    in_unit = false;
                }
            }
            else
            {
                in_unit = true;
            }
        }

        if (in_unit)
        {
            count++;
        }

        return count;
    }

    bool Neat::loadNeatParams(const std::string &file, bool output)
    {
        std::filesystem::path paramPath(file);
        std::cout << "Attempting to load: " << std::filesystem::absolute(paramPath) << std::endl;
        if (!std::filesystem::exists(paramPath))
        {
            std::cerr << "Error: File does not exist at " << std::filesystem::absolute(paramPath) << std::endl;
            return -1;
        }

        std::ifstream paramFile(file);
        if (!paramFile)
        {
            return false;
        }

        char curword[128];

        if (output)
            std::cout << "Loading NEAT parms: " << file << std::endl;

        paramFile >> curword;
        paramFile >> experiment;

        paramFile >> curword;
        paramFile >> trait_param_mut_prob;

        paramFile >> curword;
        paramFile >> linktrait_mut_sig;

        paramFile >> curword;
        paramFile >> nodetrait_mut_sig;

        paramFile >> curword;
        paramFile >> weight_mut_power;

        paramFile >> curword;
        paramFile >> recur_prob;

        paramFile >> curword;
        paramFile >> disjoint_coeff;

        paramFile >> curword;
        paramFile >> excess_coeff;

        paramFile >> curword;
        paramFile >> mutdiff_coeff;

        paramFile >> curword;
        paramFile >> compat_threshold;

        paramFile >> curword;
        paramFile >> age_significance;

        paramFile >> curword;
        paramFile >> survival_thresh;

        paramFile >> curword;
        paramFile >> mutate_only_prob;

        paramFile >> curword;
        paramFile >> mutate_random_trait_prob;

        paramFile >> curword;
        paramFile >> mutate_link_trait_prob;

        paramFile >> curword;
        paramFile >> mutate_node_trait_prob;

        paramFile >> curword;
        paramFile >> mutate_link_weights_prob;

        paramFile >> curword;
        paramFile >> mutate_toggle_enable_prob;

        paramFile >> curword;
        paramFile >> mutate_gene_reenable_prob;

        paramFile >> curword;
        paramFile >> mutate_add_node_prob;

        paramFile >> curword;
        paramFile >> mutate_add_link_prob;

        paramFile >> curword;
        paramFile >> interspecies_mate_rate;

        paramFile >> curword;
        paramFile >> mate_multipoint_prob;

        paramFile >> curword;
        paramFile >> mate_multipoint_avg_prob;

        paramFile >> curword;
        paramFile >> mate_singlepoint_prob;

        paramFile >> curword;
        paramFile >> mate_only_prob;

        paramFile >> curword;
        paramFile >> recur_only_prob;

        paramFile >> curword;
        paramFile >> pop_size;

        paramFile >> curword;
        paramFile >> dropoff_age;

        paramFile >> curword;
        paramFile >> newlink_tries;

        paramFile >> curword;
        paramFile >> print_every;

        paramFile >> curword;
        paramFile >> babies_stolen;

        paramFile >> curword;
        paramFile >> num_runs;

        if (output)
        {
            std::cout << "============== Parameters ================" << std::endl;
            std::cout << "Experiment: " << experiment << std::endl;
            std::cout << "trait_param_mut_prob: " << trait_param_mut_prob << std::endl;
            std::cout << "linktrait_mut_sig: " << linktrait_mut_sig << std::endl;
            std::cout << "nodetrait_mut_sig: " << nodetrait_mut_sig << std::endl;
            std::cout << "weight_mut_power: " << weight_mut_power << std::endl;
            std::cout << "recur_prob: " << recur_prob << std::endl;
            std::cout << "disjoint_coeff: " << disjoint_coeff << std::endl;
            std::cout << "excess_coeff: " << excess_coeff << std::endl;
            std::cout << "mutdiff_coeff: " << mutdiff_coeff << std::endl;
            std::cout << "compat_threshold: " << compat_threshold << std::endl;
            std::cout << "age_significance: " << age_significance << std::endl;
            std::cout << "survival_thresh: " << survival_thresh << std::endl;
            std::cout << "mutate_only_prob: " << mutate_only_prob << std::endl;
            std::cout << "mutate_random_trait_prob: " << mutate_random_trait_prob << std::endl;
            std::cout << "mutate_link_trait_prob: " << mutate_link_trait_prob << std::endl;
            std::cout << "mutate_node_trait_prob: " << mutate_node_trait_prob << std::endl;
            std::cout << "mutate_link_weights_prob: " << mutate_link_weights_prob << std::endl;
            std::cout << "mutate_toggle_enable_prob: " << mutate_toggle_enable_prob << std::endl;
            std::cout << "mutate_gene_reenable_prob: " << mutate_gene_reenable_prob << std::endl;
            std::cout << "mutate_add_node_prob: " << mutate_add_node_prob << std::endl;
            std::cout << "mutate_add_link_prob: " << mutate_add_link_prob << std::endl;
            std::cout << "interspecies_mate_rate: " << interspecies_mate_rate << std::endl;
            std::cout << "mate_multipoint_prob: " << mate_multipoint_prob << std::endl;
            std::cout << "mate_multipoint_avg_prob: " << mate_multipoint_avg_prob << std::endl;
            std::cout << "mate_singlepoint_prob: " << mate_singlepoint_prob << std::endl;
            std::cout << "mate_only_prob: " << mate_only_prob << std::endl;
            std::cout << "recur_only_prob: " << recur_only_prob << std::endl;
            std::cout << "pop_size: " << pop_size << std::endl;
            std::cout << "dropoff_age: " << dropoff_age << std::endl;
            std::cout << "newlink_tries: " << newlink_tries << std::endl;
            std::cout << "print_every: " << print_every << std::endl;
            std::cout << "babies_stolen: " << babies_stolen << std::endl;
            std::cout << "num_runs: " << num_runs << std::endl;
            std::cout << "==========================================" << std::endl;
        }

        paramFile.close();

        return true;
    }

    double Neat::gaussrand() const
    {
        static int iset = 0;
        static double gset;
        double fac, rsq, v1, v2;

        if (iset == 0)
        {
            do
            {
                v1 = 2.0 * (randfloat()) - 1.0;
                v2 = 2.0 * (randfloat()) - 1.0;
                rsq = v1 * v1 + v2 * v2;
            } while (rsq >= 1.0 || rsq == 0.0);
            fac = sqrt(-2.0 * log(rsq) / rsq);
            gset = v1 * fac;
            iset = 1;
            return v2 * fac;
        }
        else
        {
            iset = 0;
            return gset;
        }
    }

    double Neat::fsigmoid(double activesum, double slope, double constant) const
    {
        // RIGHT SHIFTED ---------------------------------------------------------
        // return (1/(1+(exp(-(slope*activesum-constant))))); //ave 3213 clean on 40 runs of p2m and 3468 on another 40
        // 41394 with 1 failure on 8 runs

        // LEFT SHIFTED ----------------------------------------------------------
        // return (1/(1+(exp(-(slope*activesum+constant))))); //original setting ave 3423 on 40 runs of p2m, 3729 and 1 failure also

        // PLAIN SIGMOID ---------------------------------------------------------
        // return (1/(1+(exp(-activesum)))); //3511 and 1 failure

        // LEFT SHIFTED NON-STEEPENED---------------------------------------------
        // return (1/(1+(exp(-activesum-constant)))); //simple left shifted

        // NON-SHIFTED STEEPENED
        return (1 / (1 + (exp(-(slope * activesum))))); // Compressed
    }

    double Neat::oldhebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) const
    {
        bool neg = false;
        double delta;

        // double weight_mag;

        if (maxweight < 5.0)
            maxweight = 5.0;

        if (weight > maxweight)
            weight = maxweight;

        if (weight < -maxweight)
            weight = -maxweight;

        if (weight < 0)
        {
            neg = true;
            weight = -weight;
        }

        // if (weight<0) {
        //   weight_mag=-weight;
        // }
        // else weight_mag=weight;

        if (!(neg))
        {
            // if (true) {
            delta =
                hebb_rate * (maxweight - weight) * active_in * active_out +
                pre_rate * (weight)*active_in * (active_out - 1.0) +
                post_rate * (weight) * (active_in - 1.0) * active_out;

            // delta=delta-hebb_rate/2; //decay

            // delta=delta+randposneg()*randfloat()*0.01; //noise

            // cout<<"delta: "<<delta<<endl;

            if (weight + delta > 0)
                return weight + delta;
            // else return 0.01;

            // return weight+delta;
        }
        else
        {
            // In the inhibatory case, we strengthen the synapse when output is low and
            // input is high
            delta =
                hebb_rate * (maxweight - weight) * active_in * (1.0 - active_out) + //"unhebb"
                // hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
                -5 * hebb_rate * (weight)*active_in * active_out + // anti-hebbian
                // hebb_rate*(maxweight-weight)*active_in*active_out+
                // pre_rate*weight*active_in*(active_out-1.0)+
                // post_rate*weight*(active_in-1.0)*active_out;
                0;

            // delta=delta-hebb_rate; //decay

            // delta=delta+randposneg()*randfloat()*0.01; //noise

            if (-(weight + delta) < 0)
                return -(weight + delta);
            else
                return -0.01;

            return -(weight + delta);
        }

        return 0.0;
    }

    double Neat::hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) const
    {
        bool neg = false;
        double delta;

        // double weight_mag;

        double topweight;

        if (maxweight < 5.0)
            maxweight = 5.0;

        if (weight > maxweight)
            weight = maxweight;

        if (weight < -maxweight)
            weight = -maxweight;

        if (weight < 0)
        {
            neg = true;
            weight = -weight;
        }

        // if (weight<0) {
        //   weight_mag=-weight;
        // }
        // else weight_mag=weight;

        topweight = weight + 2.0;
        if (topweight > maxweight)
            topweight = maxweight;

        if (!(neg))
        {
            // if (true) {
            delta =
                hebb_rate * (maxweight - weight) * active_in * active_out +
                pre_rate * (topweight)*active_in * (active_out - 1.0);
            // post_rate*(weight+1.0)*(active_in-1.0)*active_out;

            // delta=delta-hebb_rate/2; //decay

            // delta=delta+randposneg()*randfloat()*0.01; //noise

            // cout<<"delta: "<<delta<<endl;

            // if (weight+delta>0)
            //   return weight+delta;
            // else return 0.01;

            return weight + delta;
        }
        else
        {
            // In the inhibatory case, we strengthen the synapse when output is low and
            // input is high
            delta =
                pre_rate * (maxweight - weight) * active_in * (1.0 - active_out) + //"unhebb"
                // hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
                -hebb_rate * (topweight + 2.0) * active_in * active_out + // anti-hebbian
                // hebb_rate*(maxweight-weight)*active_in*active_out+
                // pre_rate*weight*active_in*(active_out-1.0)+
                // post_rate*weight*(active_in-1.0)*active_out;
                0;

            // delta=delta-hebb_rate; //decay

            // delta=delta+randposneg()*randfloat()*0.01; //noise

            // if (-(weight+delta)<0)
            //   return -(weight+delta);
            //   else return -0.01;

            return -(weight + delta);
        }
    }

} // namespace Neat
