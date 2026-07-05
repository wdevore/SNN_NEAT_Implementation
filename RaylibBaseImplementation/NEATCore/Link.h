#pragma once

#include <memory>
#include <vector>

#include "NNode.h"
#include "Trait.h"
#include "Neat.h"

namespace Neat
{
    class Link
    {
    private:
        /* data */
    public:
        double weight;                   // Weight of connection
        std::shared_ptr<NNode> in_node;  // NNode inputting into the link
        std::shared_ptr<NNode> out_node; // NNode that the link affects
        bool is_recurrent;
        bool time_delay;

        std::shared_ptr<Trait> linktrait; // Points to a trait of parameters for genetic creation

        int trait_id; // identify the trait derived by this link

        // ************ LEARNING PARAMETERS ***********
        // These are link-related parameters that change during Hebbian type learning

        double added_weight; // The amount of weight adjustment
        std::vector<double> params{};

        Link(/* args */);
        ~Link();

        // ================================================
        // Factories
        // ================================================
        static std::shared_ptr<Link> makeFromNodes(const Neat &neat,
                                                   double w,
                                                   std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                   bool recur);

        static std::shared_ptr<Link> makeFromTrait(const Neat &neat,
                                                   std::shared_ptr<Trait> lt,
                                                   double w,
                                                   std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                                   bool recur);

        static std::shared_ptr<Link> makeFromWeight(const Neat &neat,
                                                    double w);
        static std::shared_ptr<Link> makeCopy(const Neat &neat, const std::shared_ptr<Link> &link);

        void derive_trait(Trait *curtrait);
    };

} // namespace Neat
