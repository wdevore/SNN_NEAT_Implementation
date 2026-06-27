#include "Link.h"

namespace Neat
{
    Link::Link(/* args */)
    {
    }

    Link::Link(const Neat &neat,
               double w,
               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
               bool recur)
    {
        weight = w;
        in_node = inode;
        out_node = onode;
        is_recurrent = recur;
        added_weight = 0;
        linktrait = 0;
        time_delay = false;
        trait_id = 1;
        params.resize(neat.num_trait_params);
    }

    Link::~Link()
    {
    }

} // namespace Neat
