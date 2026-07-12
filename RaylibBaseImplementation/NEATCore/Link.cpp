#include "Link.h"

namespace Neat
{
    Link::Link(/* args */)
    {
    }

    Link::~Link()
    {
    }

    std::shared_ptr<Link> Link::makeFromNodes(const Neat &neat,
                                              double w,
                                              std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                              bool recur)
    {
        auto newLink = std::make_shared<Link>();

        newLink->weight = w;
        newLink->in_node = inode;
        newLink->out_node = onode;
        newLink->is_recurrent = recur;
        newLink->added_weight = 0;
        newLink->linktrait = 0;
        newLink->time_delay = false;
        newLink->trait_id = 1;

        newLink->params.resize(neat.num_trait_params);

        return newLink;
    }

    std::shared_ptr<Link> Link::makeFromTrait(const Neat &neat,
                                              std::shared_ptr<Trait> lt,
                                              double w,
                                              std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                              bool recur)
    {
        auto newLink = makeFromNodes(neat, w, inode, onode, recur);

        newLink->linktrait = lt;

        if (lt)
            newLink->trait_id = lt->trait_id;
        else
            newLink->trait_id = 1;

        return newLink;
    }

    std::shared_ptr<Link> Link::makeFromWeight(const Neat &neat, double w)
    {
        auto newLink = makeFromNodes(neat, w, nullptr, nullptr, false);

        return newLink;
    }

    std::shared_ptr<Link> Link::makeCopy(const Neat &neat, const std::shared_ptr<Link> &link)
    {
        auto newLink = makeFromNodes(neat, link->weight, link->in_node, link->out_node, link->is_recurrent);

        newLink->added_weight = link->added_weight;
        newLink->linktrait = link->linktrait;
        newLink->time_delay = link->time_delay;
        newLink->trait_id = link->trait_id;

        return newLink;
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
