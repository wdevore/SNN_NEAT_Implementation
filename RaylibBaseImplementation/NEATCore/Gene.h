#pragma once

#include <memory>

#include "Link.h"
#include "NNode.h"
#include "Neat.h"

namespace Neat
{
    class Gene
    {
    private:
        /* data */
    public:
        // Smart shared point of type Link
        std::shared_ptr<Link> link;
        double innovation_num;
        double mutation_num; // Used to see how much mutation has changed the link
        bool enable;         // When this is off the Gene is disabled
        bool frozen;         // When frozen, the linkweight cannot be mutated

        Gene(/* args */);
        ~Gene();

        // ================================================
        // Factories
        // ================================================
        // Construct a gene with no trait
        static std::shared_ptr<Gene> makeFromNodes(const Neat &neat,
                                                   double w,
                                                   std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                   bool recur, double innov, double mnum);

        // Construct a gene with a trait
        static std::shared_ptr<Gene> makeFromTrait(const Neat &neat,
                                                   std::shared_ptr<Trait> tp, double w,
                                                   std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                   bool recurrent, double innov, double mnum);

        static std::shared_ptr<Gene> makeFromTraitNonRecurrent(const Neat &neat,
                                                               std::shared_ptr<Trait> tp, double w,
                                                               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                               double innov, double mnum);

        static std::shared_ptr<Gene> makeFromTraitRecurrent(const Neat &neat,
                                                            std::shared_ptr<Trait> tp, double w,
                                                            std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                            double innov, double mnum);

        // Construct a gene off of another gene as a duplicate
        static std::shared_ptr<Gene> makeFromGene(const Neat &neat,
                                                  const Gene &g,
                                                  std::shared_ptr<Trait> tp,
                                                  std::shared_ptr<NNode> inode,
                                                  std::shared_ptr<NNode> onode);

        // Copy Constructor
        static std::shared_ptr<Gene> makeCopy(const Neat &neat, const Gene &gene);

        // Construct a gene from a file spec given traits and nodes
        static std::shared_ptr<Gene> makeFromLine(const Neat &neat,
                                                  const std::string argline,
                                                  std::vector<std::shared_ptr<Trait>> &traits,
                                                  std::vector<std::shared_ptr<NNode>> &nodes);

        // ================================================
        // Factories
        // ================================================

        void toFile(std::ofstream &outFile);
    };

} // namespace Neat
