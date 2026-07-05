#include "Innovation.h"

namespace Neat
{
    Innovation::Innovation(/* args */)
    {
    }

    Innovation::~Innovation()
    {
    }

    std::shared_ptr<Innovation> Innovation::makeAsNewNodeType(
        int nin, int nout,
        double num1, double num2,
        int newid, double oldinnov)
    {
        auto newInnovation = std::make_shared<Innovation>();

        newInnovation->innovation_type = NEWNODE;
        newInnovation->node_in_id = nin;
        newInnovation->node_out_id = nout;
        newInnovation->innovation_num1 = num1;
        newInnovation->innovation_num2 = num2;
        newInnovation->newnode_id = newid;
        newInnovation->old_innov_num = oldinnov;

        // Unused parameters set to zero
        newInnovation->new_weight = 0;
        newInnovation->new_traitnum = 0;
        newInnovation->recur_flag = false;

        return newInnovation;
    }

    std::shared_ptr<Innovation> Innovation::makeAsNewLinkType(int nin, int nout, double num1, double w, int t)
    {
        auto newInnovation = std::make_shared<Innovation>();

        newInnovation->innovation_type = NEWLINK;
        newInnovation->node_in_id = nin;
        newInnovation->node_out_id = nout;
        newInnovation->innovation_num1 = num1;
        newInnovation->new_weight = w;
        newInnovation->new_traitnum = t;

        // Unused parameters set to zero
        newInnovation->innovation_num2 = 0;
        newInnovation->newnode_id = 0;
        newInnovation->recur_flag = false;

        return newInnovation;
    }

    std::shared_ptr<Innovation> Innovation::makeAsNewLinkRecurrentType(int nin, int nout, double num1, double w, int t, bool recur)
    {
        auto newInnovation = std::make_shared<Innovation>();

        newInnovation->innovation_type = NEWLINK;
        newInnovation->node_in_id = nin;
        newInnovation->node_out_id = nout;
        newInnovation->innovation_num1 = num1;
        newInnovation->new_weight = w;
        newInnovation->new_traitnum = t;

        // Unused parameters set to zero
        newInnovation->innovation_num2 = 0;
        newInnovation->newnode_id = 0;
        newInnovation->recur_flag = recur;

        return newInnovation;
    }

} // namespace Neat
