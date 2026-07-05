#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Genome.h"
#include "Neat.h"

namespace Neat
{
    class NNode;

    /// @brief
    /// A NETWORK is a LIST of input NODEs and a LIST of output NODEs
    ///
    ///     The point of the network is to define a single entity which can evolve
    ///     or learn on its own, even though it may be part of a larger framework
    class Network
    {
    private:
        /* data */
    public:
        int numnodes; // The number of nodes in the net (-1 means not yet counted)
        int numlinks; // The number of links in the net (-1 means not yet counted)

        std::vector<std::shared_ptr<NNode>> all_nodes; // A list of all the nodes

        std::vector<std::shared_ptr<NNode>>::iterator input_iter; // For GUILE network inputting

        std::shared_ptr<Genome> genotype; // Allows Network to be matched with its Genome

        std::string name;                            // Every Network or subNetwork can have a name
        std::vector<std::shared_ptr<NNode>> inputs;  // NNodes that input into the network
        std::vector<std::shared_ptr<NNode>> outputs; // Values output by the network

        int net_id; // Allow for a network id

        double maxweight; // Maximum weight in network for adaptation purposes

        bool adaptable; // Tells whether network can adapt or not

        Network(/* args */);
        ~Network();

        // ================================================
        // Factories
        // ================================================
        // This constructor allows the input and output lists to be supplied
        // Defaults to not using adaptation
        static std::shared_ptr<Network> makeUnAdaptable(
            std::vector<std::shared_ptr<NNode>> in, std::vector<std::shared_ptr<NNode>> out,
            std::vector<std::shared_ptr<NNode>> all, int netid);

        // Same as previous constructor except the adaptibility can be set true or false with adaptval
        static std::shared_ptr<Network> makeAdaptable(
            std::vector<std::shared_ptr<NNode>> in,
            std::vector<std::shared_ptr<NNode>> out,
            std::vector<std::shared_ptr<NNode>> all,
            int netid, bool adaptval);

        // This constructs a net with empty input and output lists
        static std::shared_ptr<Network> makeEmpty(int netid);

        // Same as previous constructor except the adaptibility can be set true or false with adaptval
        static std::shared_ptr<Network> makeEmptyAdaptable(int netid, bool adaptval);

        // Copy Constructor
        static std::shared_ptr<Network> makeCopy(const Network &network);

        void destroy(); // Kills all nodes and links within

        // Puts the network back into an inactive state
        void flush();

        // Verify flushedness for debugging
        void flush_check();

        // Activates the net such that all outputs are active
        bool activate(const Neat &neat);

        // Add a new input node
        void add_input(std::shared_ptr<NNode> node);

        // Add a new output node
        void add_output(std::shared_ptr<NNode> node);

        // Takes an array of sensor values and loads it into SENSOR inputs ONLY
        void load_sensors(double *);
        void load_sensors(const std::vector<float> &sensvals);

        // Takes and array of output activations and OVERRIDES the outputs' actual
        // activations with these values (for adaptation)
        void override_outputs(double *);

        // Name the network
        void give_name(const std::string &name);

        // Counts the number of nodes in the net if not yet counted
        int nodecount();
        void nodecounthelper(std::shared_ptr<NNode> curnode,
                             int &counter,
                             std::vector<std::shared_ptr<NNode>> &seenlist);
        // Counts the number of links in the net if not yet counted
        int linkcount();
        void linkcounthelper(std::shared_ptr<NNode> curnode,
                             int &counter,
                             std::vector<std::shared_ptr<NNode>> &seenlist);

        // This checks a POTENTIAL link between a potential in_node
        // and potential out_node to see if it must be recurrent
        // Use count and thresh to jump out in the case of an infinite loop
        bool is_recur(std::shared_ptr<NNode> potin_node, std::shared_ptr<NNode> potout_node, int &count, int thresh);

        // Some functions to help GUILE input into Networks
        int input_start();

        int load_in(double d);

        // If all output are not active then return true
        bool outputsoff();

        int max_depth();
    };

} // namespace Neat
