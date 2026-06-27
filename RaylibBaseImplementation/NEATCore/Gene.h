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
        // Construct a gene with no trait
        Gene(const Neat &neat,
             double w, std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
             bool recur, double innov, double mnum);
        ~Gene();
    };

} // namespace Neat
