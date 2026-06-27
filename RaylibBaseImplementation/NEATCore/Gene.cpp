#include "Gene.h"

namespace Neat
{
    Gene::Gene(/* args */)
    {
    }

    Gene::Gene(const Neat &neat,
               double w,
               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
               bool recur, double innov, double mnum)
    {
        link = std::make_shared<Link>(neat, w, inode, onode, recur);
        innovation_num = innov;
        mutation_num = mnum;

        enable = true;

        frozen = false;
    }

    Gene::~Gene()
    {
    }

} // namespace Neat
