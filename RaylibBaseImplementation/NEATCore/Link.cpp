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

    Link::Link(const Neat &neat, std::shared_ptr<Trait> lt, double w, std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode, bool recur)
    {
        weight = w;
        in_node = inode;
        out_node = onode;
        is_recurrent = recur;
        added_weight = 0;
        linktrait = lt;
        time_delay = false;
        if (lt != 0)
            trait_id = lt->trait_id;
        else
            trait_id = 1;
    }

    Link::Link(double w)
    {
        weight = w;
        in_node = out_node = 0;
        is_recurrent = false;
        linktrait = 0;
        time_delay = false;
        trait_id = 1;
    }

    Link::Link(const Link &link)
    {
        weight = link.weight;
        in_node = link.in_node;
        out_node = link.out_node;
        is_recurrent = link.is_recurrent;
        added_weight = link.added_weight;
        linktrait = link.linktrait;
        time_delay = link.time_delay;
        trait_id = link.trait_id;
    }

    Link::Link(const Link *link)
    {
        weight = link->weight;
        in_node = link->in_node;
        out_node = link->out_node;
        is_recurrent = link->is_recurrent;
        added_weight = link->added_weight;
        linktrait = link->linktrait;
        time_delay = link->time_delay;
        trait_id = link->trait_id;
    }

    Link::~Link()
    {
    }

    void Link::derive_trait(Trait *curtrait)
    {
        if (curtrait)
        {
            params = curtrait->params;
            trait_id = curtrait->trait_id;
        }
        else
        {
            std::fill(params.begin(), params.end(), 0.0);
            trait_id = 1;
        }
    }

} // namespace Neat
