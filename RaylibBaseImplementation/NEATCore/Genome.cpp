#include <stdexcept>
#include <algorithm>

#include "Genome.h"

namespace Neat
{

    Genome::Genome(/* args */)
    {
    }

    // For L-values
    Genome::Genome(int id,
                   const std::vector<std::shared_ptr<Trait>> &t,
                   const std::vector<std::shared_ptr<NNode>> &n,
                   const std::vector<std::shared_ptr<Gene>> &g)
    {
        genome_id = id;
        traits = t; // Copy-assignment
        nodes = n;  // Copy-assignment
        genes = g;  // Copy-assignment
    }

    // For R-values
    Genome::Genome(int id,
                   std::vector<std::shared_ptr<Trait>> &&t,
                   std::vector<std::shared_ptr<NNode>> &&n,
                   std::vector<std::shared_ptr<Gene>> &&g)
    {
        genome_id = id;
        traits = std::move(t); // Move-assignment
        nodes = std::move(n);  // Move-assignment
        genes = std::move(g);  // Move-assignment
    }

    // For L-values
    Genome::Genome(const Neat &neat,
                   int id,
                   const std::vector<std::shared_ptr<Trait>> &t,
                   const std::vector<std::shared_ptr<NNode>> &n,
                   const std::vector<std::shared_ptr<Link>> &links)
    {
        genome_id = id;
        traits = t; // Copy-assignment
        nodes = n;  // Copy-assignment

        for (const auto &link : links)
        {
            auto gene = std::make_shared<Gene>(neat,
                                               link->linktrait,
                                               link->weight,
                                               link->in_node, link->out_node,
                                               link->is_recurrent, 1.0, 0.0);
            genes.push_back(gene);
        }
    }

    // For R-values
    Genome::Genome(const Neat &neat,
                   int id,
                   std::vector<std::shared_ptr<Trait>> &&t,
                   std::vector<std::shared_ptr<NNode>> &&n,
                   const std::vector<std::shared_ptr<Link>> &links)
    {
        genome_id = id;
        traits = std::move(t); // Move-assignment
        nodes = std::move(n);  // Move-assignment

        // We go through the links and turn them into original genes
        for (const auto &link : links)
        {
            // Create genes one at a time
            auto gene = std::make_shared<Gene>(neat,
                                               link->linktrait,
                                               link->weight,
                                               link->in_node, link->out_node,
                                               link->is_recurrent, 1.0, 0.0);
            genes.push_back(gene);
        }
    }

    Genome::Genome(const Neat &neat, const Genome &genome)
    {
        genome_id = genome.genome_id;

        for (const auto &trait : genome.traits)
        {
            auto newTrait = std::make_shared<Trait>(*trait);
            traits.push_back(newTrait);
        }

        // Duplicate NNodes
        for (const auto &node : genome.nodes)
        {
            std::shared_ptr<Trait> assoc_trait = nullptr;

            // First, find the trait that this node points to
            if (node->get_nodetrait() != nullptr)
            {
                auto it = std::find_if(traits.begin(), traits.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == node->get_nodetrait()->trait_id;
                                       });

                if (it != traits.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newnode = std::make_shared<NNode>(*node, assoc_trait);

            node->set_dup(newnode); // Remember this node's old copy
            nodes.push_back(newnode);
        }

        // Duplicate Genes
        for (const auto &gene : genome.genes)
        {
            // First find the nodes connected by the gene's link
            auto inode = gene->link->in_node->get_dup();
            auto onode = gene->link->out_node->get_dup();

            std::shared_ptr<Trait> assoc_trait = nullptr;
            // Get a pointer to the trait expressed by this gene
            if (gene->link->linktrait)
            {
                auto it = std::find_if(traits.begin(), traits.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == gene->link->linktrait->trait_id;
                                       });
                if (it != traits.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newgene = std::make_shared<Gene>(neat, *gene, assoc_trait, inode, onode);
            genes.push_back(newgene);
        }
    }

    Genome::Genome(const Neat &neat,
                   int new_id, int i, int o, int n, int nmax, bool r, double linkprob)
    {
        int totalnodes;
        bool *cm;  // The connection matrix which will be randomized
        bool *cmp; // Connection matrix pointer
        int matrixdim;
        int count;

        int ncount; // Node and connection counters
        int ccount;

        int row; // For navigating the matrix
        int col;

        double new_weight;

        int maxnode; // No nodes above this number for this genome

        int first_output; // Number of first output node

        totalnodes = i + o + nmax;
        matrixdim = totalnodes * totalnodes;
        cm = new bool[matrixdim]; // Dimension the connection matrix
        maxnode = i + n;

        first_output = totalnodes - o + 1;

        // For creating the new genes
        std::shared_ptr<NNode> newnode;
        std::shared_ptr<Gene> newgene;
        std::shared_ptr<NNode> in_node;
        std::shared_ptr<NNode> out_node;

        // Retrieves the nodes pointed to by connection genes
        std::vector<std::shared_ptr<NNode>>::iterator node_iter;

        // Assign the id
        genome_id = new_id;

        // cout<<"Assigned id "<<genome_id<<endl;

        // Step through the connection matrix, randomly assigning bits
        cmp = cm;
        for (count = 0; count < matrixdim; count++)
        {
            if (neat.randfloat() < linkprob)
                *cmp = true;
            else
                *cmp = false;
            cmp++;
        }

        // Create a dummy trait (this is for future expansion of the system)
        auto newtrait = std::make_shared<Trait>(neat, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        traits.push_back(newtrait);

        // Build the input nodes
        for (ncount = 1; ncount <= i; ncount++)
        {
            if (ncount < i)
                newnode = std::make_shared<NNode>(SENSOR, ncount, INPUT);
            else
                newnode = std::make_shared<NNode>(SENSOR, ncount, BIAS);

            newnode->setTrait(newtrait);

            // Add the node to the list of nodes
            nodes.push_back(newnode);
        }

        // Build the hidden nodes
        for (ncount = i + 1; ncount <= i + n; ncount++)
        {
            newnode = std::make_shared<NNode>(NEURON, ncount, HIDDEN);
            newnode->setTrait(newtrait);
            // Add the node to the list of nodes
            nodes.push_back(newnode);
        }

        // Build the output nodes
        for (ncount = first_output; ncount <= totalnodes; ncount++)
        {
            newnode = std::make_shared<NNode>(NEURON, ncount, OUTPUT);
            newnode->setTrait(newtrait);
            // Add the node to the list of nodes
            nodes.push_back(newnode);
        }

        // cout<<"Built nodes"<<endl;

        // Connect the nodes
        ccount = 1; // Start the connection counter

        // Step through the connection matrix, creating connection genes
        cmp = cm;
        count = 0;
        for (col = 1; col <= totalnodes; col++)
            for (row = 1; row <= totalnodes; row++)
            {
                // Only try to create a link if it is in the matrix
                // and not leading into a sensor

                if ((*cmp == true) && (col > i) &&
                    ((col <= maxnode) || (col >= first_output)) &&
                    ((row <= maxnode) || (row >= first_output)))
                {
                    // If it isn't recurrent, create the connection no matter what
                    if (col > row)
                    {

                        // Retrieve the in_node
                        node_iter = nodes.begin();
                        while ((*node_iter)->node_id != row)
                            node_iter++;

                        in_node = (*node_iter);

                        // Retrieve the out_node
                        node_iter = nodes.begin();
                        while ((*node_iter)->node_id != col)
                            node_iter++;

                        out_node = (*node_iter);

                        // Create the gene
                        new_weight = neat.randposneg() * neat.randfloat();

                        newgene = std::make_shared<Gene>(neat, newtrait, new_weight, in_node, out_node, false, count, new_weight);

                        // Add the gene to the genome
                        genes.push_back(newgene);
                    }
                    else if (r)
                    {
                        // Create a recurrent connection

                        // Retrieve the in_node
                        node_iter = nodes.begin();
                        while ((*node_iter)->node_id != row)
                            node_iter++;

                        in_node = (*node_iter);

                        // Retrieve the out_node
                        node_iter = nodes.begin();
                        while ((*node_iter)->node_id != col)
                            node_iter++;

                        out_node = (*node_iter);

                        // Create the gene
                        new_weight = neat.randposneg() * neat.randfloat();

                        newgene = std::make_shared<Gene>(neat, newtrait, new_weight, in_node, out_node, true, count, new_weight);

                        // Add the gene to the genome
                        genes.push_back(newgene);
                    }
                }

                count++; // increment gene counter
                cmp++;
            }

        delete[] cm;
    }

    Genome::~Genome()
    {
    }

    int Genome::get_last_node_id()
    {
        if (nodes.empty())
        {
            return -1; // Or handle as an error, returning -1 is an error
        }
        return nodes.back()->node_id + 1;
    }

    double Genome::get_last_gene_innovnum()
    {
        if (genes.empty())
        {
            return -1.0; // Or handle as an error, returning -1 is an error
        }
        return genes.back()->innovation_num + 1;
    }

} // namespace Neat
