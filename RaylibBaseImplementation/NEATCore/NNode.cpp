#include <iostream>
#include <algorithm>

#include "Network.h"
#include "NNode.h"
#include "Link.h"

namespace Neat
{

    NNode::NNode(/* args */)
    {
    }

    NNode::~NNode()
    {
    }

    std::shared_ptr<NNode> makeFromPlacment(Nodetype ntype, int nodeid, Nodeplace placement);

    std::shared_ptr<NNode> makeFromNeat(const Neat &neat)
    {
        auto newNode = std::make_shared<NNode>();
        newNode->params.resize(neat.num_trait_params);

        return newNode;
    }

    std::shared_ptr<NNode> makeFromType(Nodetype ntype, int nodeid)
    {
        auto newNode = makeFromPlacment(ntype, nodeid, HIDDEN);

        return newNode;
    }

    std::shared_ptr<NNode> makeFromPlacment(Nodetype ntype, int nodeid, Nodeplace placement)
    {
        auto newNode = makeFromType(ntype, nodeid);

        newNode->active_flag = false;
        newNode->activesum = 0;
        newNode->activation = 0;
        newNode->output = 0;
        newNode->last_activation = 0;
        newNode->last_activation2 = 0;
        newNode->type = ntype;         // NEURON or SENSOR type
        newNode->activation_count = 0; // Inactive upon creation
        newNode->node_id = nodeid;
        newNode->ftype = SIGMOID;
        newNode->nodetrait = 0;
        newNode->gen_node_label = placement;
        newNode->dup = 0;
        newNode->analogue = 0;
        newNode->frozen = false;
        newNode->trait_id = 1;
        newNode->override = false;

        return newNode;
    }

    // incoming and outgoing are not copied as they represent the phenotype network structure
    std::shared_ptr<NNode> NNode::makeFromTrait(const NNode &nnode, std::shared_ptr<Trait> t)
    {
        auto newNode = makeFromPlacment(nnode.type, nnode.node_id, nnode.gen_node_label);

        newNode->active_flag = nnode.active_flag;
        newNode->activesum = nnode.activesum;
        newNode->activation = nnode.activation;
        newNode->output = nnode.output;
        newNode->last_activation = nnode.last_activation;
        newNode->last_activation2 = nnode.last_activation2;

        newNode->activation_count = nnode.activation_count;

        newNode->ftype = nnode.ftype;
        newNode->nodetrait = t;
        if (t)
        {
            newNode->trait_id = t->trait_id;
        }
        else
            newNode->trait_id = 1;

        newNode->dup = nnode.dup;
        newNode->analogue = nnode.analogue;
        newNode->frozen = nnode.frozen;
        newNode->trait_id = nnode.trait_id;
        newNode->override = nnode.override;
        newNode->override_value = nnode.override_value;

        newNode->params = nnode.params;

        return newNode;
    }

    std::shared_ptr<NNode> NNode::makeCopy(const NNode &nnode)
    {
        auto newNode = std::make_shared<NNode>();

        newNode->active_flag = nnode.active_flag;
        newNode->activesum = nnode.activesum;
        newNode->activation = nnode.activation;
        newNode->output = nnode.output;
        newNode->last_activation = nnode.last_activation;
        newNode->last_activation2 = nnode.last_activation2;
        newNode->type = nnode.type;                         // NEURON or SENSOR type
        newNode->activation_count = nnode.activation_count; // Inactive upon creation
        newNode->node_id = nnode.node_id;
        newNode->ftype = nnode.ftype;
        newNode->nodetrait = nnode.nodetrait;
        newNode->gen_node_label = nnode.gen_node_label;
        newNode->dup = nnode.dup;
        newNode->analogue = nnode.dup;
        newNode->frozen = nnode.frozen;
        newNode->trait_id = nnode.trait_id;
        newNode->override = nnode.override;

        return newNode;
    }

    bool NNode::sensor_load(double value)
    {
        if (type == SENSOR)
        {

            // Time delay memory
            last_activation2 = last_activation;
            last_activation = activation;

            activation_count++; // Puts sensor into next time-step
            activation = value;
            return true;
        }
        else
            return false;
    }

    // Note: NEAT keeps track of which links are recurrent and which
    // are not even though this is unnecessary for activation.
    // It is useful to do so for 2 other reasons:
    // 1. It makes networks visualization of recurrent networks possible
    // 2. It allows genetic control of the proportion of connections
    //    that may become recurrent

    // Add an incoming connection a node
    void NNode::add_incoming(const Neat &neat, std::shared_ptr<NNode> feednode, double weight, bool recur)
    {
        auto newlink = Link::makeFromNodes(neat, weight, feednode, shared_from_this(), recur);
        incoming.push_back(newlink);
        (feednode->outgoing).push_back(newlink);
    }

    // Nonrecurrent version
    void NNode::add_incoming(const Neat &neat, std::shared_ptr<NNode> feednode, double weight)
    {
        auto newlink = Link::makeFromNodes(neat, weight, feednode, shared_from_this(), false);
        incoming.push_back(newlink);
        (feednode->outgoing).push_back(newlink);
    }

    // This recursively flushes everything leading into and including this NNode,
    // including recurrencies
    void NNode::flushback()
    {
        // A sensor should not flush black
        if (type != SENSOR)
        {
            if (activation_count > 0)
            {
                activation_count = 0;
                activation = 0;
                last_activation = 0;
                last_activation2 = 0;
            }

            // Flush back recursively
            for (const auto &link : incoming)
            {
                // Flush the link itself (For future learning parameters possibility)
                link->added_weight = 0;

                if (link->in_node->activation_count > 0)
                    link->in_node->flushback();
            }
        }
        else
        {
            // Flush the SENSOR
            activation_count = 0;
            activation = 0;
            last_activation = 0;
            last_activation2 = 0;
        }
    }

    // This recursively checks everything leading into and including this NNode,
    // including recurrencies
    // Useful for debugging
    void NNode::flushback_check(std::vector<std::shared_ptr<NNode>> &seenlist)
    {
        if (!(type == SENSOR))
        {
            // std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
            // std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
            // std::cout<<"ALERT: "<<this<<" has last_activation  "<<last_activation<<std::endl;
            // std::cout<<"ALERT: "<<this<<" has last_activation2  "<<last_activation2<<std::endl;

            if (activation_count > 0)
            {
                std::cout << "ALERT: " << this << " has activation count " << activation_count << std::endl;
            }

            if (activation > 0)
            {
                std::cout << "ALERT: " << this << " has activation  " << activation << std::endl;
            }

            if (last_activation > 0)
            {
                std::cout << "ALERT: " << this << " has last_activation  " << last_activation << std::endl;
            }

            if (last_activation2 > 0)
            {
                std::cout << "ALERT: " << this << " has last_activation2  " << last_activation2 << std::endl;
            }

            for (const auto &link : incoming)
            {
                auto location = std::find(seenlist.begin(), seenlist.end(), link->in_node);
                if (location == seenlist.end())
                {
                    seenlist.push_back(link->in_node);
                    link->in_node->flushback_check(seenlist);
                }
            }
        }
        else
        {
            // Flush_check the SENSOR
            std::cout << "sALERT: " << this << " has activation count " << activation_count << std::endl;
            std::cout << "sALERT: " << this << " has activation  " << activation << std::endl;
            std::cout << "sALERT: " << this << " has last_activation  " << last_activation << std::endl;
            std::cout << "sALERT: " << this << " has last_activation2  " << last_activation2 << std::endl;

            if (activation_count > 0)
            {
                std::cout << "ALERT: " << this << " has activation count " << activation_count << std::endl;
            }

            if (activation > 0)
            {
                std::cout << "ALERT: " << this << " has activation  " << activation << std::endl;
            }

            if (last_activation > 0)
            {
                std::cout << "ALERT: " << this << " has last_activation  " << last_activation << std::endl;
            }

            if (last_activation2 > 0)
            {
                std::cout << "ALERT: " << this << " has last_activation2  " << last_activation2 << std::endl;
            }
        }
    }

    void NNode::toFile(std::ofstream &outFile)
    {
        outFile << "node " << node_id << " ";
        if (nodetrait != 0)
            outFile << nodetrait->trait_id << " ";
        else
            outFile << "0 ";
        outFile << type << " ";
        outFile << gen_node_label << std::endl;
    }

    // Reserved for future system expansion
    void NNode::derive_trait(const Neat &neat, Trait *curtrait)
    {
        if (curtrait != 0)
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

    void NNode::override_output(double new_output)
    {
        override_value = new_output;
        override = true;
    }

    void NNode::activate_override()
    {
        activation = override_value;
        override = false;
    }

    // Find the greatest depth starting from this neuron at depth d
    int NNode::depth(int d, Network *mynet)
    {
        int cur_depth; // The depth of the current node
        int max = d;   // The max depth

        if (d > 100)
        {
            // std::cout<<mynet->genotype<<std::endl;
            // std::cout<<"** DEPTH NOT DETERMINED FOR NETWORK WITH LOOP"<<std::endl;
            return 10;
        }

        // Base Case
        if (type == SENSOR)
            return d;
        // Recursion
        else
        {
            for (const auto &curlink : incoming)
            {
                cur_depth = curlink->in_node->depth(d + 1, mynet);
                if (cur_depth > max)
                    max = cur_depth;
            }

            return max;
        }
    }

}