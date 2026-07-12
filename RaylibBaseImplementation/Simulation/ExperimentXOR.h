#pragma once

#include "Experiment.h"

namespace Neat
{

    class Organism;
    class Population;

    class ExperimentXOR : public Experiment
    {
    private:
        /* data */
    public:
        std::shared_ptr<Population> pop = std::make_shared<Population>();
        char curword[20];
        int id;
        int expcount = 0;

        std::vector<int> evals{}; // Hold records for each run
        std::vector<int> genes{};
        std::vector<int> nodes{};
        int winnernum;
        int winnergenes;
        int winnernodes;
        // For averaging
        int totalevals = 0;
        int totalgenes = 0;
        int totalnodes = 0;

        int gens = 0;

        std::ifstream iFile;

        std::shared_ptr<Genome> start_genome;

        ExperimentXOR();
        ExperimentXOR(const Neat &neat);
        ~ExperimentXOR();

        void initialize(const Neat &neat, int gens) override;

        virtual bool runStep(const Neat &neat, int gen) override;
        virtual bool run(const Neat &neat, bool paused) override;
        virtual void post_test(const Neat &neat) override;
        bool evaluate(const Neat &neat, std::shared_ptr<Organism> org);
        int epoch(const Neat &neat, std::shared_ptr<Population> pop,
                  int generation,
                  const std::string &filename,
                  int &winnernum, int &winnergenes, int &winnernodes);

        void reset() override;

        bool executeGen(const Neat &neat, int genID);
    };
} // namespace Neat