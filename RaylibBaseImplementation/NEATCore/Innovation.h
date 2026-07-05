#pragma once

#include <memory>

namespace Neat
{
    enum Innovtype
    {
        NEWNODE = 0,
        NEWLINK = 1
    };

    class Innovation
    {
    private:
        /* data */
    public:
        Innovtype innovation_type; // Either NEWNODE or NEWLINK

        int node_in_id; // Two nodes specify where the innovation took place
        int node_out_id;

        double innovation_num1; // The number assigned to the innovation
        double innovation_num2; // If this is a new node innovation, then there are 2 innovations (links) added for the new node

        double new_weight; //  If a link is added, this is its weight
        int new_traitnum;  // If a link is added, this is its connected trait

        int newnode_id; // If a new node was created, this is its node_id

        double old_innov_num; // If a new node was created, this is the innovnum of the gene's link it is being stuck inside

        bool recur_flag;

        Innovation(/* args */);
        ~Innovation();

        // ================================================
        // Factories
        // ================================================
        // Constructor for the new node case
        static std::shared_ptr<Innovation> makeAsNewNodeType(
            int nin, int nout,
            double num1, double num2,
            int newid, double oldinnov);

        // Constructor for new link case
        static std::shared_ptr<Innovation> makeAsNewLinkType(
            int nin, int nout,
            double num1,
            double w, int t);

        // Constructor for a recur link
        static std::shared_ptr<Innovation> makeAsNewLinkRecurrentType(
            int nin, int nout,
            double num1, double w,
            int t, bool recur);
    };

} // namespace Neat
