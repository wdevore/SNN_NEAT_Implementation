#pragma once

#include <vector>
#include <memory>

#include "Gene.h"
#include "Innovation.h"

namespace Neat
{
    enum Mutator
    {
        GAUSSIAN = 0,
        COLDGAUSSIAN = 1
    };

    /// @brief
    /// A Genome is the primary source of genotype information used to create
    /// a phenotype.  It contains 3 major constituents:
    ///
    ///  1) A list of Traits
    ///
    ///  2) A list of NNodes pointing to a Trait from (1)
    ///
    ///  3) A list of Genes with Links that point to Traits from (1)
    ///
    ///(1) Reserved parameter space for future use
    ///
    ///(2) NNode specifications
    ///
    ///(3) Is the primary source of innovation in the evolutionary Genome.
    ///    Each Gene in (3) has a marker telling when it arose historically.
    ///    Thus, these Genes can be used to speciate the population, and the
    ///    list of Genes provide an evolutionary history of innovation and
    ///    link-building.
    class Genome : public std::enable_shared_from_this<Genome>
    {
    private:
        // Adds a new gene that has been created through a mutation in the
        // *correct order* into the list of genes in the genome
        void add_gene(std::vector<std::shared_ptr<Gene>> &glist, std::shared_ptr<Gene> g);

        // Inserts a NNode into a given ordered list of NNodes in order
        void node_insert(std::vector<std::shared_ptr<NNode>> &nlist, std::shared_ptr<NNode> n);

    public:
        int genome_id;

        std::vector<std::shared_ptr<Trait>> traits; // parameter conglomerations
        std::vector<std::shared_ptr<NNode>> nodes;  // List of NNodes for the Network
        std::vector<std::shared_ptr<Gene>> genes;   // List of innovation-tracking genes

        // Allows Genome to be matched with its Network. Constructed in genesis()
        std::shared_ptr<Network> phenotype;

        Genome(/* args */);
        ~Genome();

        // ================================================
        // Factories
        // ================================================
        // Copy
        static std::shared_ptr<Genome> makeCopy(const Neat &neat, const Genome &genome);

        // Factory which takes full genome specs and puts them into the new one
        static std::shared_ptr<Genome> makeFromSpecs(int id,
                                                     const std::vector<std::shared_ptr<Trait>> &t,
                                                     const std::vector<std::shared_ptr<NNode>> &n,
                                                     const std::vector<std::shared_ptr<Gene>> &g);

        // Factory which takes in links (not genes) and creates a Genome
        static std::shared_ptr<Genome> makeFromLinks(const Neat &neat,
                                                     int id,
                                                     const std::vector<std::shared_ptr<Trait>> &t,
                                                     const std::vector<std::shared_ptr<NNode>> &n,
                                                     const std::vector<std::shared_ptr<Link>> &links);

        // This special constructor creates a Genome
        // with i inputs, o outputs, n out of nmax hidden units, and random
        // connectivity.  If r is true then recurrent connections will be included.
        // The last input is a bias. Linkprob is the probability of a link
        static std::shared_ptr<Genome> makeFromProbability(const Neat &neat,
                                                           int new_id,
                                                           int i, int o, int n,
                                                           int nmax, bool recurrent,
                                                           double linkprob);

        // Special constructor that creates a Genome of 3 possible types:
        // 0 - Fully linked, no hidden nodes
        // 1 - Fully linked, one hidden node splitting each link
        // 2 - Fully connected with a hidden layer, recurrent
        // num_hidden is only used in type 2
        static std::shared_ptr<Genome> makeFromTypes(const Neat &neat,
                                                     int num_in, int num_out,
                                                     int num_hidden, int type);

        int get_last_node_id();          // Return id of final NNode in Genome
        double get_last_gene_innovnum(); // Return last innovation number in Genome

        // Generate a network phenotype from this Genome with specified id
        std::shared_ptr<Network> genesis(const Neat &neat, int id);

        // Duplicate this Genome to create a new one with the specified id
        std::shared_ptr<Genome> duplicate(const Neat &neat, int new_id);

        // For debugging: A number of tests can be run on a genome to check its
        // integrity
        // Note: Some of these tests do not indicate a bug, but rather are meant
        // to be used to detect specific system states
        bool verify();

        // ******* MUTATORS *******

        // Perturb params in one trait
        void mutate_random_trait(const Neat &neat);

        // Change random link's trait. Repeat times times
        void mutate_link_trait(const Neat &neat, int times);

        // Change random node's trait times times
        void mutate_node_trait(const Neat &neat, int times);

        // Add Gaussian noise to linkweights either GAUSSIAN or COLDGAUSSIAN (from zero)
        void mutate_link_weights(const Neat &neat, double power, double rate, Mutator mut_type);

        // toggle genes on or off
        void mutate_toggle_enable(const Neat &neat, int times);

        // Find first disabled gene and enable it
        void mutate_gene_reenable();

        // These last kinds of mutations return false if they fail
        //   They can fail under certain conditions,  being unable
        //   to find a suitable place to make the mutation.
        //   Generally, if they fail, they can be called again if desired.

        // Mutate genome by adding a node respresentation
        bool mutate_add_node(
            const Neat &neat,
            std::vector<std::shared_ptr<Innovation>> &innovs,
            int &curnode_id,
            double &curinnov);

        // Mutate the genome by adding a new link between 2 random NNodes
        bool mutate_add_link(const Neat &neat,
                             std::vector<std::shared_ptr<Innovation>> &innovs,
                             double &curinnov, int tries);

        void mutate_add_sensor(const Neat &neat,
                               std::vector<std::shared_ptr<Innovation>> &innovs,
                               double &curinnov);

        // ****** MATING METHODS *****

        // This method mates this Genome with another Genome g.
        //   For every point in each Genome, where each Genome shares
        //   the innovation number, the Gene is chosen randomly from
        //   either parent.  If one parent has an innovation absent in
        //   the other, the baby will inherit the innovation
        //   Interspecies mating leads to all genes being inherited.
        //   Otherwise, excess genes come from most fit parent.
        std::shared_ptr<Genome> mate_multipoint(const Neat &neat,
                                                std::shared_ptr<Genome> g,
                                                int genomeid,
                                                double fitness1, double fitness2, bool interspec_flag);

        // This method mates like multipoint but instead of selecting one
        //    or the other when the innovation numbers match, it averages their
        //    weights
        std::shared_ptr<Genome> mate_multipoint_avg(const Neat &neat,
                                                    std::shared_ptr<Genome> g,
                                                    int genomeid,
                                                    double fitness1, double fitness2,
                                                    bool interspec_flag);

        // This method is similar to a standard single point CROSSOVER
        //   operator.  Traits are averaged as in the previous 2 mating
        //   methods.  A point is chosen in the smaller Genome for crossing
        //   with the bigger one.
        std::shared_ptr<Genome> mate_singlepoint(const Neat &neat,
                                                 std::shared_ptr<Genome> g,
                                                 int genomeid);

        // ******** COMPATIBILITY CHECKING METHODS ********

        // This function gives a measure of compatibility between
        //   two Genomes by computing a linear combination of 3
        //   characterizing variables of their compatibilty.
        //   The 3 variables represent PERCENT DISJOINT GENES,
        //   PERCENT EXCESS GENES, MUTATIONAL DIFFERENCE WITHIN
        //   MATCHING GENES.  So the formula for compatibility
        //   is:  disjoint_coeff*pdg+excess_coeff*peg+mutdiff_coeff*mdmg.
        //   The 3 coefficients are global system parameters
        double compatibility(const Neat &neat,
                             const std::shared_ptr<Genome> g);

        double trait_compare(const Trait *t1, const Trait *t2);

        // Return number of non-disabled genes
        int extrons();

        // Randomize the trait pointers of all the node and connection genes
        void randomize_traits(const Neat &neat);
    };

} // namespace Neat
