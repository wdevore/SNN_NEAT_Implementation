#pragma once

#include <memory>
#include <fstream>

#include "Trait.h"

namespace Neat
{
    enum Nodetype
    {
        NEURON = 0,
        SENSOR = 1
    };

    enum Nodeplace
    {
        HIDDEN = 0,
        INPUT = 1,
        OUTPUT = 2,
        BIAS = 3
    };

    enum Functype
    {
        SIGMOID = 0
    };

    // Forward declarations because Link and NNode are circular dependent on each other
    class Link;
    class Network;

    /// @brief
    /// A NODE is either a NEURON or a SENSOR.
    ///
    ///  - If it's a sensor, it can be loaded with a value for output
    ///
    ///  - If it's a neuron, it has a list of its incoming input signals (List<Link> is used)
    ///
    /// Use an activation count to avoid flushing
    class NNode : public std::enable_shared_from_this<NNode>
    {
    private:
        /* data */

    public:
        int activation_count;    // keeps track of which activation the node is currently in
        double last_activation;  // Holds the previous step's activation for recurrency
        double last_activation2; // Holds the activation BEFORE the prevous step's

        // This is necessary for a special recurrent case when the innode
        // of a recurrent link is one time step ahead of the outnode.
        // The innode then needs to send from TWO time steps ago

        std::shared_ptr<Trait> nodetrait; // Points to a trait of parameters

        int trait_id; // identify the trait derived by this node

        std::shared_ptr<NNode> dup; // Used for Genome duplication

        std::shared_ptr<NNode> analogue; // Used for Gene decoding

        bool override; // The NNode cannot compute its own output- something is overriding it

        double override_value; // Contains the activation value that will override this node's activation

        bool frozen; // When frozen, cannot be mutated (meaning its trait pointer is fixed)

        Functype ftype; // type is either SIGMOID ..or others that can be added
        Nodetype type;  // type is either NEURON or SENSOR

        double activesum;  // The incoming activity before being processed
        double activation; // The total activation entering the NNode
        bool active_flag;  // To make sure outputs are active

        // NOT USED IN NEAT - covered by "activation" above
        double output; // Output of the NNode- the value in the NNode

        // ************ LEARNING PARAMETERS ***********
        // The following parameters are for use in
        //   neurons that learn through habituation,
        //   sensitization, or Hebbian-type processes

        std::vector<double> params{};

        // TODO THESE MAY NEED TO WEAK POINTERS
        std::vector<std::shared_ptr<Link>> incoming; // A list of pointers to incoming weighted signals from other nodes
        std::vector<std::shared_ptr<Link>> outgoing; // A list of pointers to links carrying this node's signal

        // These members are used for graphing with GTK+/GDK
        std::vector<double> rowlevels; // Depths from output where this node appears
        int row;                       // Final row decided upon for drawing this NNode in
        int ypos;
        int xpos;

        int node_id; // A node can be given an identification number for saving in files

        Nodeplace gen_node_label; // Used for genetic marking of nodes

        NNode();
        ~NNode();

        // ================================================
        // Factories
        // ================================================
        static std::shared_ptr<NNode> makeFromNeat(const Neat &neat);
        static std::shared_ptr<NNode> makeFromType(Nodetype ntype, int nodeid);

        static std::shared_ptr<NNode> makeFromPlacment(Nodetype ntype, int nodeid, Nodeplace placement);
        // Copy based on Trait
        static std::shared_ptr<NNode> makeFromTrait(const NNode &nnode, std::shared_ptr<Trait> t);
        static std::shared_ptr<NNode> makeCopy(const NNode &nnode);
        static std::shared_ptr<NNode> makeFromLine(const Neat &neat, const std::string &argline, std::vector<std::shared_ptr<Trait>> &t);

        // ================================================
        // Methods
        // ================================================
        bool isSensor() { return (type == SENSOR); }
        bool isNeuron() { return (type == NEURON); }

        void setTrait(const std::shared_ptr<Trait> &t) { nodetrait = t; }

        // Return activation currently in node, if it has been activated, for step
        double get_active_out() { return (activation_count > 0) ? activation : 0.0; }

        // Return activation currently in node from PREVIOUS (time-delayed) time step,
        // if there is one
        double get_active_out_td() { return (activation_count > 1) ? last_activation : 0.0; }

        // Returns the type of the node, NEURON or SENSOR
        const Nodetype get_type() { return type; }

        // Allows alteration between NEURON and SENSOR.  Returns its argument
        void set_type(Nodetype nType) { type = nType; }

        // If the node is a SENSOR, returns true and loads the value
        bool sensor_load(double);

        // Adds a Link to a new NNode in the incoming List
        void add_incoming(const Neat &neat, std::shared_ptr<NNode>, double, bool);

        // Adds a NONRECURRENT Link to a new NNode in the incoming List
        void add_incoming(const Neat &neat, std::shared_ptr<NNode>, double);

        // Recursively deactivate backwards through the network
        void flushback();

        // Verify flushing for debugging
        void flushback_check(std::vector<std::shared_ptr<NNode>> &seenlist);

        void toFile(std::ofstream &outFile);

        // Have NNode gain its properties from the trait
        void derive_trait(const Neat &neat, Trait *curtrait);

        // Returns the gene that created the node
        std::shared_ptr<NNode> get_analogue() { return analogue; }

        std::shared_ptr<NNode> get_dup() { return dup; }
        void set_dup(std::shared_ptr<NNode> new_dup) { dup = new_dup; }
        std::shared_ptr<Trait> get_nodetrait() const { return nodetrait; }

        // Force an output value on the node
        void override_output(double new_output);

        // Tell whether node has been overridden
        bool overridden() { return override; }

        // Set activation to the override value and turn off override
        void activate_override();

        // Find the greatest depth starting from this neuron at depth d
        int depth(int d, Network *mynet);
    };

} // namespace Neat
