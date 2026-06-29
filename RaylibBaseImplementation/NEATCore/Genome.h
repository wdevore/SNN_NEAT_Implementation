#pragma once

#include <vector>
#include <memory>

#include "Gene.h"

namespace Neat
{
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
    class Genome
    {
    private:
        /* data */
    public:
        int genome_id;

        std::vector<std::shared_ptr<Trait>> traits; // parameter conglomerations
        std::vector<std::shared_ptr<NNode>> nodes;  // List of NNodes for the Network
        std::vector<std::shared_ptr<Gene>> genes;   // List of innovation-tracking genes

        // Allows Genome to be matched with its Network. Constructed in genesis()
        std::unique_ptr<Network> phenotype;

        Genome(/* args */);
        // Constructor which takes full genome specs and puts them into the new one
        Genome(int id,
               const std::vector<std::shared_ptr<Trait>> &t,
               const std::vector<std::shared_ptr<NNode>> &n,
               const std::vector<std::shared_ptr<Gene>> &g);

        // Constructor for moving from rvalues
        Genome(int id,
               std::vector<std::shared_ptr<Trait>> &&t,
               std::vector<std::shared_ptr<NNode>> &&n,
               std::vector<std::shared_ptr<Gene>> &&g);

        // Constructor which takes in links (not genes) and creates a Genome
        Genome(const Neat &neat,
               int id,
               const std::vector<std::shared_ptr<Trait>> &t,
               const std::vector<std::shared_ptr<NNode>> &n,
               const std::vector<std::shared_ptr<Link>> &links);
        Genome(const Neat &neat,
               int id,
               std::vector<std::shared_ptr<Trait>> &&t,
               std::vector<std::shared_ptr<NNode>> &&n,
               const std::vector<std::shared_ptr<Link>> &links);
        // Copy constructor
        Genome(const Neat &neat, const Genome &genome);

        // This special constructor creates a Genome
        // with i inputs, o outputs, n out of nmax hidden units, and random
        // connectivity.  If r is true then recurrent connections will
        // be included.
        // The last input is a bias
        // Linkprob is the probability of a link
        Genome(const Neat &neat,
               int new_id, int i, int o, int n, int nmax, bool r, double linkprob);

        ~Genome();

        int get_last_node_id();          // Return id of final NNode in Genome
        double get_last_gene_innovnum(); // Return last innovation number in Genome
    };

} // namespace Neat
