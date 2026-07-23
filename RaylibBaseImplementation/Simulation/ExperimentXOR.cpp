#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <cmath>

#include "ExperimentXOR.h"
#include "Population.h"
#include "Organism.h"

namespace Neat
{
    ExperimentXOR::ExperimentXOR()
    {
    }

    ExperimentXOR::ExperimentXOR(const Neat &neat)
    {
        evals.resize(neat.num_runs);
        genes.resize(neat.num_runs);
        nodes.resize(neat.num_runs);
    }

    ExperimentXOR::~ExperimentXOR()
    {
    }

    void ExperimentXOR::initialize(Neat &neat, int gens)
    {
        iFile.open("assets/xorstartgenes");

        this->gens = gens;

        // Read in the start Genome
        iFile >> curword;
        iFile >> id;
        neat.log("Read in Genome id ", id);

        start_genome = Genome::makeFromFile(neat, id, iFile);

        iFile.close();

        // Spawn the Population
        neat.log("Spawning Population off Genome2");

        neat.disableLog();
        pop = Population::makeFromGenome(neat, start_genome, neat.pop_size);
        neat.enableLog();

        neat.log("Verifying Spawned Pop");
        pop->verify();
    }

    bool ExperimentXOR::runStep(const Neat &neat, int gen)
    {
        // Spawn the Population and run it
        // for (int expcount = 0; expcount < neat.num_runs; expcount++)
        // {
        // // Spawn the Population
        // neat.log("Spawning Population off Genome2");

        // pop = Population::makeFromGenome(neat, start_genomeg, neat.pop_size);

        // neat.log("Verifying Spawned Pop");
        // pop->verify();

        // for (int gen = 1; gen <= gens; gen++)
        // {
        bool winnerFound = executeGen(neat, gen);
        // }
        // }

        expcount++;

        return winnerFound;
    }

    bool ExperimentXOR::run(const Neat &neat, bool paused)
    {
        bool winner{false};

        if (!paused)
        {
            winner = runStep(neat, generationCnt);
            generationCnt++;
        }

        return winner;
    }

    void ExperimentXOR::post_test(const Neat &neat)
    {
        // Average and print stats
        // neat.log("Nodes: ");
        // for (const auto &node_count : nodes)
        // {
        //     neat.log(node_count);
        //     totalnodes += node_count;
        // }

        // neat.log("Genes: ");
        // for (const auto &gene_count : genes)
        // {
        //     neat.log(gene_count);
        //     totalgenes += gene_count;
        // }

        // neat.log("Evals ");
        // int samples = 0;
        // for (const auto &eval_count : evals)
        // {
        //     neat.log(eval_count);
        //     if (eval_count > 0)
        //     {
        //         totalevals += eval_count;
        //         samples++;
        //     }
        // }

        // neat.log("Failures: " << (neat.num_runs - samples) << " out of " << neat.num_runs << " runs");
        // neat.log("Average Nodes: " << (samples > 0 ? (double)totalnodes / samples : 0));
        // neat.log("Average Genes: " << (samples > 0 ? (double)totalgenes / samples : 0));
        // neat.log("Average Evals: " << (samples > 0 ? (double)totalevals / samples : 0));
    }

    bool ExperimentXOR::evaluate(const Neat &neat, std::shared_ptr<Organism> org)
    {
        double out[4]; // The four outputs
        // double this_out; // The current output
        double errorsum;

        bool success; // Check for successful activation
        int numnodes; // Used to figure out how many nodes
                      // should be visited during activation

        int net_depth; // The max depth of the network to be activated
        int relax;     // Activates until relaxation

        // The four possible input combinations to xor
        // The first number is for biasing
        double in[4][3] = {{1.0, 0.0, 0.0},
                           {1.0, 0.0, 1.0},
                           {1.0, 1.0, 0.0},
                           {1.0, 1.0, 1.0}};

        std::shared_ptr<Network> net = org->net;

        numnodes = org->gnome->nodes.size();
        // neat.log("How many nodes to visit: " << numnodes);

        net_depth = net->max_depth();
        // neat.log("Evaluate Network DEPTH: " << net_depth);

        // Load and activate the network on each input
        for (int count = 0; count <= 3; count++)
        {
            net->load_sensors(in[count]);

            // Relax net and get output
            success = net->activate(neat);

            // use depth to ensure relaxation
            for (relax = 0; relax <= net_depth; relax++)
            {
                success = net->activate(neat);
                // this_out = output_start->activation;
            }

            auto output_start = *(net->outputs.begin());

            out[count] = output_start->activation;

            net->flush();
        }

        if (success)
        {
            errorsum = (std::fabs(out[0]) + std::fabs(1.0 - out[1]) + std::fabs(1.0 - out[2]) + std::fabs(out[3]));
            org->fitness = std::pow(4.0 - errorsum, 2);
            org->error = errorsum;
            neat.log("Evaluate: new fitness: ", org->fitness);
        }
        else
        {
            // The network is flawed (shouldn't happen)
            errorsum = 999.0;
            org->fitness = neat.organism_fitness_measure;
            neat.log("Evaluate: The network is flawed (shouldn't happen)");
        }

        // neat.log("Org Genome Id " << org->gnome->genome_id << "                                     error: " << errorsum << " Outputs: [" << out[0] << " " << out[1] << " " << out[2] << " " << out[3] << "]");
        // neat.log("Org Genome Id " << org->gnome->genome_id << "                                     fitness: " << org->fitness);

        if ((out[0] < 0.5) && (out[1] >= 0.5) && (out[2] >= 0.5) && (out[3] < 0.5))
        {
            org->winner = true;
            return true;
        }
        else
        {
            org->winner = false;
            return false;
        }
    }

    int ExperimentXOR::epoch(const Neat &neat,
                             std::shared_ptr<Population> pop,
                             int generation,
                             const std::string &filename,
                             int &winnernum, int &winnergenes, int &winnernodes)
    {
        bool win = false;

        // Evaluate each organism on a test
        for (const auto &org : pop->organisms)
        {
            if (evaluate(neat, org))
            {
                win = true;
                winnernum = org->gnome->genome_id;
                winnergenes = org->gnome->extrons();
                winnernodes = org->gnome->nodes.size();
                if (winnernodes == 5)
                {
                    // You could dump out optimal genomes here if desired
                    //(*curorg)->gnome->print_to_filename("xor_optimal");
                    // cout<<"DUMPED OPTIMAL"<<endl;
                }
            }
        }

        // Average and max their fitnesses for dumping to file and snapshot
        for (const auto &s : pop->species)
        {
            // This experiment control routine issues commands to collect ave
            // and max fitness, as opposed to having the snapshot do it,
            // because this allows flexibility in terms of what time
            // to observe fitnesses at

            s->compute_average_fitness();
            s->compute_max_fitness();
        }

        // Take a snapshot of the population, so that it can be
        // visualized later on
        // if ((generation%1)==0)
        //   pop->snapshot();

        // Only print to file every print_every generations
        // if (win || ((generation % neat.print_every) == 0))
        //     pop->print_to_file_by_species(filename);

        if (win)
        {
            for (const auto &org : pop->organisms)
            {
                if (org->winner)
                {
                    neat.log("WINNER IS # ", org->gnome->genome_id);
                    // Prints the winner to file
                    // IMPORTANT: This causes generational file output!
                    // print_Genome_tofile((*curorg)->gnome, "xor_winner");
                }
            }
        }

        pop->epoch(neat, generation);

        return win ? 1 : 0;
    }

    bool ExperimentXOR::executeGen(const Neat &neat, int genID)
    {
        bool winnerFound = false;

        neat.log("=============================================");
        neat.log("=== Epoch Beginning: ", genID);
        neat.log("=============================================");
        const std::string fileName = "gen_" + std::to_string(genID);

        // Check for success
        if (epoch(neat, pop, genID, fileName, winnernum, winnergenes, winnernodes))
        {
            // Collect Stats on end of experiment
            evals[expcount] = neat.pop_size * (genID - 1) + winnernum;
            genes[expcount] = winnergenes;
            nodes[expcount] = winnernodes;
            winnerFound = true;
        }

        neat.log("=============================================");
        neat.log("=== Epoch Complete: ", genID);
        neat.log("=============================================");

        return winnerFound;
    }

    void ExperimentXOR::reset()
    {
        expcount = 0;
    }
} // namespace Neat
