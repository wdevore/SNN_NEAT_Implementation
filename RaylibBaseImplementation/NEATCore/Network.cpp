#include <algorithm>
#include <iostream>

#include "Network.h"

namespace Neat
{

    Network::Network(/* args */)
    {
    }

    Network::~Network()
    {
    }

    std::shared_ptr<Network> Network::makeUnAdaptable(
        std::vector<std::shared_ptr<NNode>> in,
        std::vector<std::shared_ptr<NNode>> out,
        std::vector<std::shared_ptr<NNode>> all, int netid)
    {
        return makeAdaptable(in, out, all, netid, false);
    }

    std::shared_ptr<Network> Network::makeAdaptable(
        std::vector<std::shared_ptr<NNode>> in,
        std::vector<std::shared_ptr<NNode>> out,
        std::vector<std::shared_ptr<NNode>> all,
        int netid, bool adaptval)
    {
        auto newNetwork = std::make_unique<Network>();

        newNetwork->inputs = in;
        newNetwork->outputs = out;
        newNetwork->all_nodes = all;
        newNetwork->name = ""; // Defaults to no name  ..NOTE: TRYING TO PRINT AN EMPTY NAME CAN CAUSE A CRASH
        newNetwork->numnodes = -1;
        newNetwork->numlinks = -1;
        newNetwork->net_id = netid;
        newNetwork->adaptable = adaptval;

        return newNetwork;
    }

    std::shared_ptr<Network> Network::makeEmpty(int netid)
    {
        return makeEmptyAdaptable(netid, false);
    }

    std::shared_ptr<Network> Network::makeEmptyAdaptable(int netid, bool adaptval)
    {
        auto newNetwork = std::make_unique<Network>();

        newNetwork->name = ""; // Defaults to no name
        newNetwork->numnodes = -1;
        newNetwork->numlinks = -1;
        newNetwork->net_id = netid;
        newNetwork->adaptable = adaptval;

        return newNetwork;
    }

    std::shared_ptr<Network> Network::makeCopy(const Network &network)
    {
        auto newNetwork = std::make_unique<Network>();

        // Copy all the inputs
        for (const auto &curnode : network.inputs)
        {
            auto n = NNode::makeCopy(*curnode);
            newNetwork->inputs.push_back(n);
            newNetwork->all_nodes.push_back(n);
        }

        // Copy all the outputs
        for (const auto &curnode : network.outputs)
        {
            auto n = NNode::makeCopy(*curnode);
            newNetwork->outputs.push_back(n);
            newNetwork->all_nodes.push_back(n);
        }

        if (!network.name.empty())
            newNetwork->name = network.name;
        else
            newNetwork->name = "";

        newNetwork->numnodes = network.numnodes;
        newNetwork->numlinks = network.numlinks;
        newNetwork->net_id = network.net_id;
        newNetwork->adaptable = network.adaptable;

        return newNetwork;
    }

    // Destroy will find every node in the network and subsequently
    // delete them one by one.  Since deleting a node deletes its incoming
    // links, all nodes and links associated with a network will be destructed
    // Note: Traits are parts of genomes and not networks, so they are not
    //       deleted here
    void Network::destroy()
    {
        // Erase all nodes from all_nodes list
        all_nodes.clear();
    }

    // Puts the network back into an initial state
    void Network::flush()
    {
        for (const auto &curnode : outputs)
        {
            curnode->flushback();
        }
    }

    // Debugger: Checks network state
    void Network::flush_check()
    {
        std::vector<std::shared_ptr<NNode>> seenlist; // List of nodes not to doublecount

        for (const auto &curnode : outputs)
        {
            auto it = std::find(seenlist.begin(), seenlist.end(), curnode);
            if (it == seenlist.end())
            {
                seenlist.push_back(curnode);
                curnode->flushback_check(seenlist);
            }
        }
    }

    // If all output are not active then return true
    bool Network::outputsoff()
    {
        for (const auto &curnode : outputs)
        {
            if (curnode->activation_count == 0)
                return true;
        }

        return false;
    }

    bool Network::is_recur(std::shared_ptr<NNode> potin_node, std::shared_ptr<NNode> potout_node, int &count, int thresh)
    {
        ++count; // Count the node as visited

        if (count > thresh)
        {
            // cout<<"returning false"<<endl;
            return false; // Short out the whole thing- loop detected
        }

        if (potin_node == potout_node)
            return true;
        else
        {
            // Check back on all links...
            for (const auto &curlink : potin_node->incoming)
            {
                // But skip links that are already recurrent
                //(We want to check back through the forward flow of signals only
                if (!curlink->is_recurrent)
                {
                    if (is_recur(curlink->in_node, potout_node, count, thresh))
                        return true;
                }
            }
            return false;
        }
    }

    // Activates the net such that all outputs are active
    // Returns true on success;
    bool Network::activate(const Neat &neat)
    {
        double add_amount{0}; // For adding to the activesum
        bool onetime{false};  // Make sure we at least activate once
        int abortcount = 0;   // Used in case the output is somehow truncated from the network

        // cout<<"Activating network: "<<this->genotype<<endl;

        // Keep activating until all the outputs have become active
        //(This only happens on the first activation, because after that they
        //  are always active)

        while (outputsoff() || !onetime)
        {
            ++abortcount;

            if (abortcount == neat.network_abort_count)
            {
                return false;
                std::cout << "Inputs disconnected from output!" << std::endl;
            }
            // std::cout<<"Outputs are off"<<std::endl;

            // For each node, compute the sum of its incoming activation
            for (const auto &curnode : all_nodes)
            {
                // Ignore SENSORS
                if (!curnode->isSensor())
                {
                    curnode->activesum = 0;
                    curnode->active_flag = false; // This will tell us if it has any active inputs

                    // For each incoming connection, add the activity from the connection to the activesum
                    for (const auto &curlink : curnode->incoming)
                    {
                        // Handle possible time delays
                        if (!curlink->time_delay)
                        {
                            add_amount = curlink->weight * curlink->in_node->get_active_out();

                            if (curlink->in_node->active_flag || curlink->in_node->isSensor())
                                curnode->active_flag = true;
                            // std::cout << "1)Node " << curnode->node_id << " adding " << add_amount << " from node " << curlink->in_node->node_id << std::endl;

                            curnode->activesum += add_amount;
                        }
                        else
                        {
                            // Input over a time delayed connection
                            add_amount = curlink->weight * curlink->in_node->get_active_out_td();
                            curnode->activesum += add_amount;
                            std::cout << "2)Node " << curnode->node_id << " adding " << add_amount << " from node " << curlink->in_node->node_id << std::endl;
                        }

                    } // End for over incoming links

                } // End if (curnode->type != SENSOR)

            } // End for over all nodes

            // Now activate all the non-sensor nodes off their incoming activation
            for (const auto &curnode : all_nodes)
            {
                if (!curnode->isSensor())
                {
                    // Only activate if some active input came in
                    if (curnode->active_flag)
                    {
                        // cout<<"Activating "<<(*curnode)->node_id<<" with "<<(*curnode)->activesum<<": ";

                        // Keep a memory of activations for potential time delayed connections
                        curnode->last_activation2 = curnode->last_activation;
                        curnode->last_activation = curnode->activation;

                        // If the node is being overrided from outside,
                        // stick in the override value
                        if (curnode->overridden())
                        {
                            // Set activation to the override value and turn off override
                            curnode->activate_override();
                        }
                        else
                        {
                            // Now run the net activation through an activation function
                            if (curnode->ftype == SIGMOID)
                                curnode->activation = neat.fsigmoid(
                                    curnode->activesum,
                                    neat.network_activate_sigmoid_slope,
                                    neat.network_activate_sigmoid_constant); // Sigmoidal activation- see comments under fsigmoid
                        }
                        // std::cout << "node activation: " << curnode->activation << std::endl;

                        // Increment the activation_count
                        // First activation cannot be from nothing!!
                        curnode->activation_count++;
                    }
                }
            }

            onetime = true;
        }

        if (adaptable)
        {
            // std::cout << "ADAPTING" << std:endl;

            // ADAPTATION:  Adapt weights based on activations
            for (const auto &curnode : all_nodes)
            {
                // Ignore SENSORS

                // cout<<"On node "<<(*curnode)->node_id<<endl;

                if (!curnode->isSensor())
                {
                    // For each incoming connection, perform adaptation based on the trait of the connection
                    for (const auto &curlink : curnode->incoming)
                    {

                        if ((curlink->trait_id == 2) ||
                            (curlink->trait_id == 3) ||
                            (curlink->trait_id == 4))
                        {

                            // In the recurrent case we must take the last activation of the input for calculating hebbian changes
                            if (curlink->is_recurrent)
                            {
                                curlink->weight = neat.hebbian(curlink->weight, maxweight,
                                                               curlink->in_node->last_activation,
                                                               curlink->out_node->get_active_out(),
                                                               curlink->params[0], curlink->params[1],
                                                               curlink->params[2]);
                            }
                            else
                            { // non-recurrent case
                                curlink->weight = neat.hebbian(curlink->weight, maxweight,
                                                               curlink->in_node->get_active_out(),
                                                               curlink->out_node->get_active_out(),
                                                               curlink->params[0], curlink->params[1],
                                                               curlink->params[2]);
                            }
                        }
                    }
                }
            }

        } // end if (adaptable)

        return true;
    }

    // Add an input
    void Network::add_input(std::shared_ptr<NNode> in_node)
    {
        inputs.push_back(in_node);
    }

    int Network::input_start()
    {
        input_iter = inputs.begin();
        return 1;
    }

    int Network::load_in(double d)
    {
        (*input_iter)->sensor_load(d);
        input_iter++;
        if (input_iter == inputs.end())
            return 0;
        else
            return 1;
    }

    // Find the maximum number of neurons between an ouput and an input
    int Network::max_depth()
    {
        int max = 0; // The max depth

        for (const auto &curoutput : outputs)
        {
            int cur_depth = curoutput->depth(0, this);
            if (cur_depth > max)
                max = cur_depth;
        }

        return max;
    }

    // Takes an array of sensor values and loads it into SENSOR inputs ONLY
    void Network::load_sensors(double *sensvals)
    {
        for (const auto &sensPtr : inputs)
        {
            // only load values into SENSORS (not BIASes)
            if (sensPtr->isSensor())
            {
                sensPtr->sensor_load(*sensvals);
                sensvals++;
            }
        }
    }

    void Network::load_sensors(const std::vector<double> &sensvals)
    {
        size_t sensor_idx = 0;
        for (const auto &input_node : inputs)
        {
            // only load values into SENSORS (not BIASes)
            if (input_node->isSensor())
            {
                if (sensor_idx < sensvals.size())
                {
                    input_node->sensor_load(sensvals[sensor_idx++]);
                }
            }
        }
    }

    // Takes and array of output activations and OVERRIDES
    // the outputs' actual activations with these values (for adaptation)
    void Network::override_outputs(double *outvals)
    {
        for (const auto &outPtr : outputs)
        {
            outPtr->override_output(*outvals);
            outvals++;
        }
    }

    void Network::give_name(const std::string &name)
    {
        this->name = name;
    }

    // The following two methods recurse through a network from outputs
    // down in order to count the number of nodes and links in the network.
    // This can be useful for debugging genotype->phenotype spawning
    // (to make sure their counts correspond)

    int Network::nodecount()
    {
        int counter = 0;
        std::vector<std::shared_ptr<NNode>> seenlist; // List of nodes not to doublecount

        for (const auto &curnode : outputs)
        {
            auto location = std::find(seenlist.begin(), seenlist.end(), curnode);
            if (location == seenlist.end())
            {
                counter++;
                seenlist.push_back(curnode);
                nodecounthelper(curnode, counter, seenlist);
            }
        }

        numnodes = counter;

        return counter;
    }

    void Network::nodecounthelper(
        std::shared_ptr<NNode> curnode,
        int &counter,
        std::vector<std::shared_ptr<NNode>> &seenlist)
    {
        if (curnode->type != SENSOR)
        {
            for (const auto &curlink : curnode->incoming)
            {
                auto location = std::find(seenlist.begin(), seenlist.end(), curlink->in_node);
                if (location == seenlist.end())
                {
                    counter++;
                    seenlist.push_back(curlink->in_node);
                    nodecounthelper(curlink->in_node, counter, seenlist);
                }
            }
        }
    }

    int Network::linkcount()
    {
        int counter = 0;
        std::vector<std::shared_ptr<NNode>> seenlist; // List of nodes not to doublecount

        for (const auto &curnode : outputs)
        {
            linkcounthelper(curnode, counter, seenlist);
        }

        numlinks = counter;

        return counter;
    }

    void Network::linkcounthelper(
        std::shared_ptr<NNode> curnode,
        int &counter,
        std::vector<std::shared_ptr<NNode>> &seenlist)
    {
        auto location = std::find(seenlist.begin(), seenlist.end(), curnode);
        if ((!((curnode->type) == SENSOR)) && (location == seenlist.end()))
        {
            seenlist.push_back(curnode);

            for (const auto &curlink : curnode->incoming)
            {
                counter++;
                linkcounthelper(curlink->in_node, counter, seenlist);
            }
        }
    }

} // namespace Neat
