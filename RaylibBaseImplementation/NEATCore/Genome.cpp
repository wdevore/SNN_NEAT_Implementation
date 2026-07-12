#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include "Genome.h"
#include "Network.h"
#include "NNode.h"

namespace Neat
{
    Genome::Genome(/* args */)
    {
    }

    Genome::~Genome()
    {
    }

    std::shared_ptr<Genome> Genome::makeCopy(const Neat &neat, const Genome &genome)
    {
        auto newGenome = std::make_shared<Genome>();

        newGenome->genome_id = genome.genome_id;

        for (const auto &trait : genome.traits)
        {
            auto newTrait = std::make_shared<Trait>(*trait);
            newGenome->traits.push_back(newTrait);
        }

        // Duplicate NNodes
        for (const auto &node : genome.nodes)
        {
            std::shared_ptr<Trait> assoc_trait = nullptr;

            // First, find the trait that this node points to
            if (node->get_nodetrait() != nullptr)
            {
                auto it = std::find_if(newGenome->traits.begin(), newGenome->traits.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == node->get_nodetrait()->trait_id;
                                       });

                if (it != newGenome->traits.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newnode = NNode::makeFromTrait(*node, assoc_trait);

            node->set_dup(newnode); // Remember this node's old copy
            newGenome->nodes.push_back(newnode);
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
                auto it = std::find_if(newGenome->traits.begin(), newGenome->traits.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == gene->link->linktrait->trait_id;
                                       });
                if (it != newGenome->traits.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newgene = Gene::makeFromGene(neat, *gene, assoc_trait, inode, onode);
            newGenome->genes.push_back(newgene);
        }

        return newGenome;
    }

    std::shared_ptr<Genome> Genome::makeFromSpecs(int id,
                                                  const std::vector<std::shared_ptr<Trait>> &t,
                                                  const std::vector<std::shared_ptr<NNode>> &n,
                                                  const std::vector<std::shared_ptr<Gene>> &g)
    {
        auto newGenome = std::make_shared<Genome>();
        newGenome->genome_id = id;
        newGenome->traits = t;
        newGenome->nodes = n;
        newGenome->genes = g;
        return newGenome;
    }

    std::shared_ptr<Genome> Genome::makeFromLinks(const Neat &neat,
                                                  int id,
                                                  const std::vector<std::shared_ptr<Trait>> &t,
                                                  const std::vector<std::shared_ptr<NNode>> &n,
                                                  const std::vector<std::shared_ptr<Link>> &links)
    {
        auto newGenome = std::make_shared<Genome>();
        newGenome->genome_id = id;
        newGenome->traits = std::move(t); // Move-assignment
        newGenome->nodes = std::move(n);  // Move-assignment

        // We go through the links and turn them into original genes
        for (const auto &link : links)
        {
            // Create genes one at a time
            auto gene = Gene::makeFromTrait(neat,
                                            link->linktrait,
                                            link->weight,
                                            link->in_node, link->out_node,
                                            link->is_recurrent, 1.0, 0.0);
            newGenome->genes.push_back(gene);
        }

        return newGenome;
    }

    std::shared_ptr<Genome> Genome::makeFromProbability(const Neat &neat,
                                                        int new_id,
                                                        int i, int o, int n,
                                                        int nmax, bool recurrent, double linkprob)
    {
        auto newGenome = std::make_shared<Genome>();

        int totalnodes;
        std::vector<bool> cm{}; // The connection matrix which will be randomized
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
        cm.resize(matrixdim);
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
        newGenome->genome_id = new_id;

        // cout<<"Assigned id "<<genome_id<<endl;

        // Step through the connection matrix, randomly assigning bits
        for (auto &&cmi : cm)
        {
            if (neat.randfloat() < linkprob)
                cmi = true;
            else
                cmi = false;
        }

        // Create a dummy trait (this is for future expansion of the system)
        auto newtrait = Trait::makeFromParams(neat, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        newGenome->traits.push_back(newtrait);

        // Build the input nodes
        for (ncount = 1; ncount <= i; ncount++)
        {
            if (ncount < i)
                newnode = NNode::makeFromPlacment(SENSOR, ncount, INPUT);
            else
                newnode = NNode::makeFromPlacment(SENSOR, ncount, BIAS);

            newnode->setTrait(newtrait);

            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
        }

        // Build the hidden nodes
        for (ncount = i + 1; ncount <= i + n; ncount++)
        {
            newnode = NNode::makeFromPlacment(NEURON, ncount, HIDDEN);
            newnode->setTrait(newtrait);
            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
        }

        // Build the output nodes
        for (ncount = first_output; ncount <= totalnodes; ncount++)
        {
            newnode = NNode::makeFromPlacment(NEURON, ncount, OUTPUT);
            newnode->setTrait(newtrait);
            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
        }

        // cout<<"Built nodes"<<endl;

        // Connect the nodes
        ccount = 1; // Start the connection counter

        // Step through the connection matrix, creating connection genes
        count = 0;
        for (col = 1; col <= totalnodes; col++)
            for (row = 1; row <= totalnodes; row++)
            {
                // Only try to create a link if it is in the matrix
                // and not leading into a sensor

                if ((cm[count] == true) && (col > i) &&
                    ((col <= maxnode) || (col >= first_output)) &&
                    ((row <= maxnode) || (row >= first_output)))
                {
                    auto in_node_it = std::find_if(newGenome->nodes.begin(), newGenome->nodes.end(),
                                                   [row](const std::shared_ptr<NNode> &n)
                                                   { return n->node_id == row; });

                    auto out_node_it = std::find_if(newGenome->nodes.begin(), newGenome->nodes.end(),
                                                    [col](const std::shared_ptr<NNode> &n)
                                                    { return n->node_id == col; });

                    if (in_node_it != newGenome->nodes.end() && out_node_it != newGenome->nodes.end())
                    {
                        in_node = *in_node_it;
                        out_node = *out_node_it;

                        // If it isn't recurrent, create the connection no matter what
                        bool re_current = (col <= row) && recurrent;
                        new_weight = neat.randposneg() * neat.randfloat();

                        newgene = Gene::makeFromTrait(neat, newtrait, new_weight, in_node, out_node, re_current, count, new_weight);

                        newGenome->genes.push_back(newgene);
                    }
                }

                count++; // increment gene counter
            }

        return newGenome;
    }

    std::shared_ptr<Genome> Genome::makeFromTypes(const Neat &neat,
                                                  int num_in, int num_out,
                                                  int num_hidden, int type)
    {
        auto newGenome = std::make_shared<Genome>();

        // Temporary lists of nodes
        std::vector<std::shared_ptr<NNode>> inputs;
        std::vector<std::shared_ptr<NNode>> outputs;
        std::vector<std::shared_ptr<NNode>> hidden;
        std::shared_ptr<NNode> bias; // Remember the bias

        // For creating the new genes
        std::shared_ptr<NNode> newnode;
        std::shared_ptr<Gene> newgene;
        std::shared_ptr<Trait> newtrait;

        int innovation;
        int ncount;
        double weight = 0.0;
        int mutation_num = 0;

        // Assign the id 0
        newGenome->genome_id = 0;

        // Create a dummy trait (this is for future expansion of the system)
        newtrait = Trait::makeFromParams(neat, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        newGenome->traits.push_back(newtrait);

        // Adjust hidden number
        num_hidden = type == 0 ? 0 : num_in * num_out;

        // Create the inputs and outputs

        // Build the input nodes
        for (ncount = 1; ncount <= num_in; ncount++)
        {
            if (ncount < num_in)
                newnode = NNode::makeFromPlacment(SENSOR, ncount, INPUT);
            else
            {
                newnode = NNode::makeFromPlacment(SENSOR, ncount, BIAS);
                bias = newnode;
            }

            // newnode->nodetrait=newtrait;

            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
            inputs.push_back(newnode);
        }

        // Build the hidden nodes
        for (ncount = num_in + 1; ncount <= num_in + num_hidden; ncount++)
        {
            newnode = NNode::makeFromPlacment(NEURON, ncount, HIDDEN);
            // newnode->nodetrait=newtrait;
            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
            hidden.push_back(newnode);
        }

        // Build the output nodes
        for (ncount = num_in + num_hidden + 1; ncount <= num_in + num_hidden + num_out; ncount++)
        {
            newnode = NNode::makeFromPlacment(NEURON, ncount, OUTPUT);
            // newnode->nodetrait=newtrait;
            // Add the node to the list of nodes
            newGenome->nodes.push_back(newnode);
            outputs.push_back(newnode);
        }

        // Create the links depending on the type
        if (type == 0)
        {
            // Just connect inputs straight to outputs

            innovation = 1;

            // Loop over the outputs
            for (const auto &output_node : outputs)
            {
                // Loop over the inputs
                for (const auto &input_node : inputs)
                {
                    // Connect each input to each output
                    newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                              newtrait, weight,
                                                              input_node, output_node,
                                                              innovation, mutation_num);

                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    innovation++;
                }
            }

        } // end type 0
        // ===___===___===___===___===___===___===___===___===___===___
        // A split link from each input to each output
        else if (type == 1)
        {
            innovation = 1; // Start the gene number counter

            auto hidden_node = hidden.begin(); // One hidden for ever input-output pair

            // Loop over the outputs
            for (const auto &output_node : outputs)
            {
                // Loop over the inputs
                for (const auto &input_node : inputs)
                {

                    // Connect Input to hidden
                    newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                              newtrait, weight,
                                                              input_node, output_node,
                                                              innovation, mutation_num);

                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    innovation++; // Next gene

                    // Connect hidden to output
                    newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                              newtrait, weight,
                                                              *hidden_node, output_node,
                                                              innovation, mutation_num);
                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    ++hidden_node; // Next hidden node
                    innovation++;  // Next gene (new innovation)
                }
            }

        } // end type 1
        // ===___===___===___===___===___===___===___===___===___===___
        // Fully connected
        else if (type == 2)
        {
            innovation = 1; // Start gene counter at 1

            // ==========================================
            // Connect all inputs to all hidden nodes
            // ==========================================
            for (const auto &hidden_node : hidden)
            {
                // Loop over the inputs
                for (const auto &input_node : inputs)
                {
                    // Connect each input to each hidden
                    newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                              newtrait, weight,
                                                              input_node, hidden_node,
                                                              innovation, mutation_num);

                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    innovation++;
                }
            }

            // ==========================================
            // Connect all hidden units to all outputs
            // ==========================================
            for (const auto &output_node : outputs)
            {
                // Loop over the inputs
                for (const auto &hidden_node : hidden)
                {
                    // Connect each input to each hidden
                    newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                              newtrait, weight,
                                                              hidden_node, output_node,
                                                              innovation, mutation_num);

                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    innovation++;
                }
            }

            // ==========================================
            // Connect the bias to all outputs
            // ==========================================
            for (const auto &output_node : outputs)
            {
                newgene = Gene::makeFromTraitNonRecurrent(neat,
                                                          newtrait, weight,
                                                          bias, output_node,
                                                          innovation, mutation_num);

                // Add the gene to the genome
                newGenome->genes.push_back(newgene);

                innovation++;
            }

            // ==========================================
            // Recurrently connect the hidden nodes
            // ==========================================
            for (const auto &hidden_node1 : hidden)
            {
                // Loop Over all Hidden
                for (const auto &hidden_node2 : hidden)
                {
                    // Connect each hidden to each hidden
                    newgene = Gene::makeFromTraitRecurrent(neat,
                                                           newtrait, weight,
                                                           hidden_node2, hidden_node1,
                                                           innovation, mutation_num);

                    // Add the gene to the genome
                    newGenome->genes.push_back(newgene);

                    innovation++;
                }
            }

        } // end type 2

        return newGenome;
    }

    std::shared_ptr<Genome> Genome::makeFromFile(const Neat &neat,
                                                 int id, std::ifstream &iFile)
    {
        auto newGenome = std::make_shared<Genome>();

        std::string curline;

        newGenome->genome_id = id;

        // Loop until file is finished, parsing each line
        while (std::getline(iFile, curline))
        {
            std::cout << curline << std::endl;

            std::stringstream ss(curline);
            std::string curword;
            ss >> curword;

            if (curword.empty())
                continue; // Skip empty lines

            printf("%s test\n", curword.c_str());

            // Check for end of Genome
            if (curword == "genomeend")
            {
                int idcheck;
                ss >> idcheck;
                if (idcheck != newGenome->genome_id)
                {
                    std::cout << "ERROR: id mismatch in genome. Got: " << idcheck << " Expected: " << newGenome->genome_id << std::endl;
                }
                break;
            }

            // Ignore genomestart if it hasn't been gobbled yet
            else if (curword == "genomestart")
            {
                std::cout << "genomestart" << std::endl;
            }

            // Ignore comments surrounded by - they get printed to screen
            else if (curword == "/*")
            {
                // strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
                while (ss >> curword && curword != "*/")
                {
                    // std::cout << curword << " ";
                    // strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
                }
                // std::cout << std::endl;
            }

            // Read in a trait
            else if (curword == "trait")
            {
                std::string argline;
                // strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

                std::getline(ss, argline);

                // Allocate the new trait
                auto newtrait = Trait::makeFromLine(neat, argline);

                // Add the trait to the list of traits
                newGenome->traits.push_back(newtrait);
            }

            // Read in a node
            else if (curword == "node")
            {
                std::string argline;
                // strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

                std::getline(ss, argline);
                // Allocate the new node
                auto newnode = NNode::makeFromLine(neat, argline, newGenome->traits);

                // Add the node to the list of nodes
                newGenome->nodes.push_back(newnode);
            }

            // Read in a Gene
            else if (curword == "gene")
            {
                std::string argline;
                // strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

                std::getline(ss, argline);
                // std::cout << "New gene: " << ss.str() << std::endl;
                // Allocate the new Gene
                auto newgene = Gene::makeFromLine(neat, argline, newGenome->traits, newGenome->nodes);

                // Add the gene to the genome
                newGenome->genes.push_back(newgene);

                // std::cout<<"Added gene " << newgene << std::endl;
            }
        }

        return newGenome;
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

    std::shared_ptr<Network> Genome::genesis(const Neat &neat, int id)
    {
        double maxweight = 0.0; // Compute the maximum weight for adaptation purposes

        // Inputs and outputs will be collected here for the network
        // All nodes are collected in an all_list-
        // this will be used for later safe destruction of the net
        std::vector<std::shared_ptr<NNode>> inlist;
        std::vector<std::shared_ptr<NNode>> outlist;
        std::vector<std::shared_ptr<NNode>> all_list;

        // Create the nodes
        for (const auto &curnode : nodes)
        {
            auto newnode = NNode::makeFromType(curnode->type, curnode->node_id);

            // Derive the node parameters from the trait pointed to
            auto curtrait = curnode->nodetrait;
            newnode->derive_trait(neat, curtrait.get());

            // Check for input or output designation of node
            if ((curnode->gen_node_label) == INPUT)
                inlist.push_back(newnode);
            else if ((curnode->gen_node_label) == BIAS)
                inlist.push_back(newnode);
            else if ((curnode->gen_node_label) == OUTPUT)
                outlist.push_back(newnode);

            // Keep track of all nodes, not just input and output
            all_list.push_back(newnode);

            // Have the node specifier point to the node it generated
            curnode->analogue = newnode;
        }

        // Create the links by iterating through the genes
        for (const auto &curgene : genes)
        {
            // Only create the link if the gene is enabled
            if (curgene->enable)
            {
                auto curlink = curgene->link;
                auto inode = curlink->in_node->analogue;
                auto onode = curlink->out_node->analogue;
                // NOTE: This line could be run through a recurrency check if desired
                //  (no need to in the current implementation of NEAT)
                auto newlink = Link::makeFromNodes(neat, curlink->weight, inode, onode, curlink->is_recurrent);

                (onode->incoming).push_back(newlink);
                (inode->outgoing).push_back(newlink);

                newlink->derive_trait(curlink->linktrait.get());

                // Keep track of maximum weight
                double weight_mag = std::abs(newlink->weight);
                if (weight_mag > maxweight)
                    maxweight = weight_mag;
            }
        }

        // The new network
        auto newnet = Network::makeUnAdaptable(inlist, outlist, all_list, id);

        // Attach genotype and phenotype together
        newnet->genotype = shared_from_this(); // A Network can still hold a shared_ptr to its Genome
        phenotype = newnet;

        newnet->maxweight = maxweight;

        return newnet;
    }

    std::shared_ptr<Genome> Genome::duplicate(const Neat &neat, int new_id)
    {
        // Collections for the new Genome
        std::vector<std::shared_ptr<Trait>> traits_dup;
        std::vector<std::shared_ptr<NNode>> nodes_dup;
        std::vector<std::shared_ptr<Gene>> genes_dup;

        // Duplicate the traits
        for (const auto &trait : traits)
        {
            auto newtrait = Trait::makeCopy(*trait);
            traits_dup.push_back(newtrait);
        }

        // Duplicate NNodes
        for (const auto &node : nodes)
        {
            std::shared_ptr<Trait> assoc_trait;
            // First, find the trait that this node points to
            if (node->nodetrait)
            {
                auto it = std::find_if(traits_dup.begin(), traits_dup.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == node->nodetrait->trait_id;
                                       });
                if (it != traits_dup.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newnode = NNode::makeFromTrait(*node, assoc_trait);

            node->dup = newnode; // Remember this node's old copy
            nodes_dup.push_back(newnode);
        }

        // Duplicate Genes
        for (const auto &gene : genes)
        {
            std::shared_ptr<Trait> assoc_trait;
            // First find the nodes connected by the gene's link
            auto inode = gene->link->in_node->dup;
            auto onode = gene->link->out_node->dup;

            // Get a pointer to the trait expressed by this gene
            auto traitptr = gene->link->linktrait;
            if (traitptr)
            {
                auto it = std::find_if(traits_dup.begin(), traits_dup.end(),
                                       [&](const std::shared_ptr<Trait> &t)
                                       {
                                           return t->trait_id == traitptr->trait_id;
                                       });
                if (it != traits_dup.end())
                {
                    assoc_trait = *it;
                }
            }

            auto newgene = Gene::makeFromGene(neat, *gene, assoc_trait, inode, onode);
            genes_dup.push_back(newgene);
        }

        // Finally, return the genome
        auto newgenome = Genome::makeFromSpecs(new_id, traits_dup, nodes_dup, genes_dup);

        return newgenome;
    }

    bool Genome::verify()
    {
        // Check each gene's nodes
        for (const auto &gene : genes)
        {
            auto inode = gene->link->in_node;
            auto onode = gene->link->out_node;

            // Look for inode
            if (std::find(nodes.begin(), nodes.end(), inode) == nodes.end())
            {
                return false;
            }

            // Look for onode
            if (std::find(nodes.begin(), nodes.end(), onode) == nodes.end())
            {
                return false;
            }
        }

        // Check for NNodes being out of order
        int last_id = 0;
        for (const auto &node : nodes)
        {
            if (node->node_id < last_id)
            {
                return false;
            }
            last_id = node->node_id;
        }

        // Make sure there are no duplicate genes
        for (const auto &gene1 : genes)
        {
            for (const auto &gene2 : genes)
            {
                if (gene1 != gene2 &&
                    (gene1->link->is_recurrent == gene2->link->is_recurrent) &&
                    (gene1->link->in_node->node_id == gene2->link->in_node->node_id) &&
                    (gene1->link->out_node->node_id == gene2->link->out_node->node_id))
                {
                    // Duplicate gene found, which might indicate an issue.
                    // Depending on the desired strictness, you might return false here.
                    std::cout << "Duplicate Gene found: in(" << gene1->link->in_node->node_id << ") out(" << gene1->link->out_node->node_id << ")" << std::endl;
                }
            }
        }

        // Check for 2 disables in a row
        // Note:  Again, this is not necessarily a bad sign
        if (nodes.size() >= 500)
        {
            bool prev_disabled = false;
            for (const auto &gene : genes)
            {
                if (!gene->enable && prev_disabled)
                {
                    // cout<<"ALERT: 2 DISABLES IN A ROW: "<<this<<endl;
                }
                prev_disabled = !gene->enable;
            }
        }

        // std::cout << "GENOME (" << genome_id << ") verified OK!" << std::endl;

        return true;
    }

    void Genome::mutate_random_trait(const Neat &neat)
    {
        // Choose a random traitnum
        int traitnum = neat.randint(0, (traits.size()) - 1);

        // Retrieve the trait and mutate it. Trait to be mutated
        auto thetrait = traits.begin();
        (*(thetrait[traitnum])).mutate(neat);

        // TRACK INNOVATION? (future possibility)
    }

    void Genome::mutate_link_trait(const Neat &neat, int times)
    {
        for (int loop = 1; loop <= times; loop++)
        {
            // Choose a random traitnum for attachment
            int traitnum = neat.randint(0, (traits.size()) - 1);
            auto &thetrait = traits.at(traitnum);

            // Choose a random gene to mutate
            int genenum = neat.randint(0, genes.size() - 1);
            auto &thegene = genes.at(genenum);

            // Do not alter frozen genes
            if (!thegene->frozen)
            {
                thegene->link->linktrait = thetrait; // Attachment
            }
        }
    }

    void Genome::mutate_node_trait(const Neat &neat, int times)
    {
        for (int loop = 1; loop <= times; loop++)
        {
            // Choose a random node number
            int nodenum = neat.randint(0, nodes.size() - 1);
            auto &thenode = nodes.at(nodenum);

            // Do not mutate frozen nodes
            if (!thenode->frozen)
            {
                // Choose a random trait number
                int traitnum = neat.randint(0, (traits.size()) - 1);
                auto &thetrait = traits.at(traitnum);

                // set the trait to point to the new trait
                thenode->nodetrait = thetrait;
            }
        }
    }

    void Genome::mutate_link_weights(const Neat &neat, double power, double rate, Mutator mut_type)
    {
        const bool severe = neat.randfloat() > 0.5;

        // Go through all the Genes and perturb their link's weights
        double num = 0.0;
        const double gene_total = (double)genes.size();
        const double endpart = gene_total * 0.8;
        // powermod=randposneg()*power*randfloat();  //Make power of mutation random
        // powermod=randfloat();
        const double powermod = 1.0;

        // Loop on all genes  (ORIGINAL METHOD)
        for (const auto &curgene : genes)
        {
            // Possibility: Have newer genes mutate with higher probability
            // Only make mutation power vary along genome if it's big enough
            // if (gene_total>=10.0) {
            // This causes the mutation power to go up towards the end up the genome
            // powermod=((power-0.7)/gene_total)*num+0.7;
            // }
            // else powermod=power;

            // The following if determines the probabilities of doing cold gaussian
            // mutation, meaning the probability of replacing a link weight with
            // another, entirely random weight.  It is meant to bias such mutations
            // to the tail of a genome, because that is where less time-tested genes
            // reside.  The gausspoint and coldgausspoint represent values above
            // which a random float will signify that kind of mutation.

            // Don't mutate weights of frozen links
            if (!curgene->frozen)
            {
                double gausspoint;
                double coldgausspoint;
                if (severe)
                {
                    gausspoint = 0.3;
                    coldgausspoint = 0.1;
                }
                else if ((gene_total >= 10.0) && (num > endpart))
                {
                    gausspoint = 0.5;     // Mutate by modification % of connections
                    coldgausspoint = 0.3; // Mutate the rest by replacement % of the time
                }
                else
                {
                    // Half the time don't do any cold mutations
                    if (neat.randfloat() > 0.5)
                    {
                        gausspoint = 1.0 - rate;
                        coldgausspoint = 1.0 - rate - 0.1;
                    }
                    else
                    {
                        gausspoint = 1.0 - rate;
                        coldgausspoint = 1.0 - rate;
                    }
                }

                // Possible methods of setting the perturbation:
                // randnum=gaussrand()*powermod;
                // randnum=gaussrand();

                double randnum = neat.randposneg() * neat.randfloat() * power * powermod;
                // std::cout << "RANDOM: " << randnum << " " << randposneg() << " " << randfloat() << " " << power << " " << powermod << std::endl;
                if (mut_type == GAUSSIAN)
                {
                    double randchoice = neat.randfloat();
                    if (randchoice > gausspoint)
                        curgene->link->weight += randnum;
                    else if (randchoice > coldgausspoint)
                        curgene->link->weight = randnum;
                }
                else if (mut_type == Mutator::COLDGAUSSIAN)
                    curgene->link->weight = randnum;

                // Cap the weights at 8.0 (experimental)
                if (curgene->link->weight > 8.0)
                    curgene->link->weight = 8.0;
                else if (curgene->link->weight < -8.0)
                    curgene->link->weight = -8.0;

                // Record the innovation
                //(*curgene)->mutation_num+=randnum;
                curgene->mutation_num = curgene->link->weight;

                num += 1.0;
            }

        } // end for loop
    }

    void Genome::mutate_toggle_enable(const Neat &neat, int times)
    {
        for (int count = 1; count <= times; count++)
        {
            // Choose a random genenum
            int genenum = neat.randint(0, genes.size() - 1);
            auto &thegene = genes.at(genenum); // Gene to toggle

            // Toggle the enable on this gene
            if (thegene->enable)
            {
                // We need to make sure that another gene connects out of the in-node
                // Because if not a section of network will break off and become isolated
                auto checkgene = std::find_if(genes.begin(), genes.end(), // Gene to check
                                              [&](const std::shared_ptr<Gene> &g)
                                              {
                                                  return g->link->in_node == thegene->link->in_node &&
                                                         g->enable &&
                                                         g->innovation_num != thegene->innovation_num;
                                              });

                // Disable the gene if it's safe to do so
                if (checkgene != genes.end())
                    thegene->enable = false;
            }
            else
                thegene->enable = true;
        }
    }

    void Genome::mutate_gene_reenable()
    {
        // Find the first disabled gene
        auto thegene_it = std::find_if(genes.begin(), genes.end(),
                                       [](const std::shared_ptr<Gene> &g)
                                       {
                                           return !g->enable;
                                       });

        // If a disabled gene is found, re-enable it
        if (thegene_it != genes.end())
        {
            (*thegene_it)->enable = true;
        }
    }

    bool Genome::mutate_add_node(
        const Neat &neat,
        std::vector<std::shared_ptr<Innovation>> &innovs,
        int &curnode_id,
        double &curinnov)
    {
        std::vector<std::shared_ptr<Gene>>::iterator thegene; // random gene containing the original link
        int genenum;                                          // The random gene number
        std::shared_ptr<NNode> in_node;                       // Here are the nodes connected by the gene
        std::shared_ptr<NNode> out_node;
        std::shared_ptr<Link> thelink; // The link inside the random gene

        // double randmult;  //using a gaussian to find the random gene

        std::vector<std::shared_ptr<Innovation>>::iterator theinnov; // For finding a historical match
        bool done = false;

        std::shared_ptr<Gene> newgene1; // The new Genes
        std::shared_ptr<Gene> newgene2;
        std::shared_ptr<NNode> newnode;  // The new NNode
        std::shared_ptr<Trait> traitptr; // The original link's trait

        // double splitweight;  //If used, Set to sqrt(oldweight of oldlink)
        double oldweight; // The weight of the original link

        int trycount; // Take a few tries to find an open node
        bool found;

        // First, find a random gene already in the genome
        trycount = 0;
        found = false;

        // Split next link with a bias towards older links
        // NOTE: 7/2/01 - for robots, went back to random split
        //         because of large # of inputs
        if (false)
        {
            thegene = genes.begin();
            while (((thegene != genes.end()) && (!((*thegene)->enable))) ||
                   ((thegene != genes.end()) && (((*thegene)->link->in_node)->gen_node_label == BIAS)))
                ++thegene;

            // Now randomize which node is chosen at this point
            // We bias the search towards older genes because
            // this encourages splitting to distribute evenly
            while (((thegene != genes.end()) &&
                    (neat.randfloat() < 0.3)) ||
                   ((thegene != genes.end()) && (((*thegene)->link->in_node)->gen_node_label == BIAS)))
            {
                ++thegene;
            }

            if ((!(thegene == genes.end())) &&
                ((*thegene)->enable))
            {
                found = true;
            }
        }
        // In this else:
        // Alternative random gaussian choice of genes NOT USED in this
        // version of NEAT
        // NOTE: 7/2/01 now we use this after all
        else
        {
            while ((trycount < 20) && (!found))
            {

                // Choose a random genenum
                // randmult=gaussrand()/4;
                // if (randmult>1.0) randmult=1.0;

                // This tends to select older genes for splitting
                // genenum=(int) floor((randmult*(genes.size()-1.0))+0.5);

                // This old totally random selection is bad- splitting
                // inside something recently splitted adds little power
                // to the system (should use a gaussian if doing it this way)
                genenum = neat.randint(0, genes.size() - 1);

                // find the gene
                thegene = genes.begin();
                for (int genecount = 0; genecount < genenum; genecount++)
                    ++thegene;

                // If either the gene is disabled, or it has a bias input, try again
                if (!(((*thegene)->enable == false) ||
                      (((((*thegene)->link)->in_node)->gen_node_label) == BIAS)))
                    found = true;

                ++trycount;
            }
        }

        // If we couldn't find anything so say goodbye
        if (!found)
            return false;

        // Disabled the gene
        (*thegene)->enable = false;

        // Extract the link
        thelink = (*thegene)->link;
        oldweight = (*thegene)->link->weight;

        // Extract the nodes
        in_node = thelink->in_node;
        out_node = thelink->out_node;

        // Check to see if this innovation has already been done
        // in another genome
        // Innovations are used to make sure the same innovation in
        // two separate genomes in the same generation receives
        // the same innovation number.
        theinnov = innovs.begin();

        while (!done)
        {

            if (theinnov == innovs.end())
            {

                // The innovation is totally novel

                // Get the old link's trait
                traitptr = thelink->linktrait;

                // Create the new NNode
                // By convention, it will point to the first trait
                newnode = NNode::makeFromPlacment(NEURON, curnode_id++, HIDDEN);
                newnode->nodetrait = (*(traits.begin()));

                // Create the new Genes
                if (thelink->is_recurrent)
                {
                    newgene1 = Gene::makeFromTrait(neat, traitptr, 1.0, in_node, newnode, true, curinnov, 0);
                    newgene2 = Gene::makeFromTrait(neat, traitptr, oldweight, newnode, out_node, false, curinnov + 1, 0);
                    curinnov += 2.0;
                }
                else
                {
                    newgene1 = Gene::makeFromTrait(neat, traitptr, 1.0, in_node, newnode, false, curinnov, 0);
                    newgene2 = Gene::makeFromTrait(neat, traitptr, oldweight, newnode, out_node, false, curinnov + 1, 0);
                    curinnov += 2.0;
                }

                // Add the innovations (remember what was done)
                auto newInnov = Innovation::makeAsNewNodeType(
                    in_node->node_id, out_node->node_id,
                    curinnov - 2.0, curinnov - 1.0,
                    newnode->node_id, (*thegene)->innovation_num);
                innovs.push_back(newInnov);

                done = true;
            }

            // We check to see if an innovation already occured that was:
            //   -A new node
            //   -Stuck between the same nodes as were chosen for this mutation
            //   -Splitting the same gene as chosen for this mutation
            //   If so, we know this mutation is not a novel innovation
            //   in this generation
            //   so we make it match the original, identical mutation which occured
            //   elsewhere in the population by coincidence
            else if (((*theinnov)->innovation_type == NEWNODE) &&
                     ((*theinnov)->node_in_id == (in_node->node_id)) &&
                     ((*theinnov)->node_out_id == (out_node->node_id)) &&
                     ((*theinnov)->old_innov_num == (*thegene)->innovation_num))
            {

                // Here, the innovation has been done before

                // Get the old link's trait
                traitptr = thelink->linktrait;

                // Create the new NNode
                newnode = NNode::makeFromPlacment(NEURON, (*theinnov)->newnode_id, HIDDEN);
                // By convention, it will point to the first trait
                // Note: In future may want to change this
                newnode->nodetrait = (*(traits.begin()));

                // Create the new Genes
                if (thelink->is_recurrent)
                {
                    newgene1 = Gene::makeFromTrait(neat, traitptr, 1.0, in_node, newnode, true, (*theinnov)->innovation_num1, 0);
                    newgene2 = Gene::makeFromTrait(neat, traitptr, oldweight, newnode, out_node, false, (*theinnov)->innovation_num2, 0);
                }
                else
                {
                    newgene1 = Gene::makeFromTrait(neat, traitptr, 1.0, in_node, newnode, false, (*theinnov)->innovation_num1, 0);
                    newgene2 = Gene::makeFromTrait(neat, traitptr, oldweight, newnode, out_node, false, (*theinnov)->innovation_num2, 0);
                }

                done = true;
            }
            else
                ++theinnov;
        }

        // Now add the new NNode and new Genes to the Genome
        add_gene(genes, newgene1); // Add genes in correct order
        add_gene(genes, newgene2);
        node_insert(nodes, newnode);

        return true;
    }

    void Genome::add_gene(std::vector<std::shared_ptr<Gene>> &glist, std::shared_ptr<Gene> g)
    {
        // Use std::lower_bound to find the correct insertion point
        // to keep the gene list sorted by innovation number.
        auto it = std::lower_bound(glist.begin(), glist.end(), g,
                                   [](const std::shared_ptr<Gene> &a, const std::shared_ptr<Gene> &b)
                                   {
                                       return a->innovation_num < b->innovation_num;
                                   });

        glist.insert(it, g);
    }

    void Genome::node_insert(std::vector<std::shared_ptr<NNode>> &nlist, std::shared_ptr<NNode> n)
    {
        // Use std::lower_bound to find the correct insertion point
        // to keep the node list sorted by node_id.
        auto it = std::lower_bound(nlist.begin(), nlist.end(), n,
                                   [](const std::shared_ptr<NNode> &a, const std::shared_ptr<NNode> &b)
                                   {
                                       return a->node_id < b->node_id;
                                   });

        nlist.insert(it, n);
    }

    bool Genome::mutate_add_link(const Neat &neat,
                                 std::vector<std::shared_ptr<Innovation>> &innovs,
                                 double &curinnov, int tries)
    {
        int nodenum1, nodenum2;                                           // Random node numbers
        std::vector<std::shared_ptr<NNode>>::iterator thenode1, thenode2; // Random node iterators
        int nodecount;                                                    // Counter for finding nodes
        int trycount;                                                     // Iterates over attempts to find an unconnected pair of nodes
        std::shared_ptr<NNode> nodep1;                                    // Pointers to the nodes
        std::shared_ptr<NNode> nodep2;                                    // Pointers to the nodes
        std::vector<std::shared_ptr<Gene>>::iterator thegene;             // Searches for existing link
        bool found = false;                                               // Tells whether an open pair was found
        std::vector<std::shared_ptr<Innovation>>::iterator theinnov;      // For finding a historical match
        int recurflag;                                                    // Indicates whether proposed link is recurrent
        std::shared_ptr<Gene> newgene;                                    // The new Gene

        int traitnum; // Random trait finder
        std::vector<std::shared_ptr<Trait>>::iterator thetrait;

        double newweight; // The new weight for the new link

        bool done;
        bool do_recur;
        bool loop_recur;
        int first_nonsensor;

        // These are used to avoid getting stuck in an infinite loop checking
        // for recursion
        // Note that we check for recursion to control the frequency of
        // adding recurrent links rather than to prevent any paricular
        // kind of error
        int thresh = (nodes.size()) * (nodes.size());
        int count = 0;

        // Make attempts to find an unconnected pair
        trycount = 0;

        // Decide whether to make this recurrent
        if (neat.randfloat() < neat.recur_only_prob)
            do_recur = true;
        else
            do_recur = false;

        // Find the first non-sensor so that the to-node won't look at sensors as
        // possible destinations
        first_nonsensor = 0;
        thenode1 = nodes.begin();
        while (((*thenode1)->get_type()) == SENSOR)
        {
            first_nonsensor++;
            ++thenode1;
        }

        // Here is the recurrent finder loop- it is done separately
        if (do_recur)
        {
            while (trycount < tries)
            {

                // Some of the time try to make a recur loop
                if (neat.randfloat() > 0.5)
                {
                    loop_recur = true;
                }
                else
                    loop_recur = false;

                if (loop_recur)
                {
                    nodenum1 = neat.randint(first_nonsensor, nodes.size() - 1);
                    nodenum2 = nodenum1;
                }
                else
                {
                    // Choose random nodenums
                    nodenum1 = neat.randint(0, nodes.size() - 1);
                    nodenum2 = neat.randint(first_nonsensor, nodes.size() - 1);
                }

                // Find the first node
                thenode1 = nodes.begin();
                for (nodecount = 0; nodecount < nodenum1; nodecount++)
                    ++thenode1;

                // Find the second node
                thenode2 = nodes.begin();
                for (nodecount = 0; nodecount < nodenum2; nodecount++)
                    ++thenode2;

                nodep1 = (*thenode1);
                nodep2 = (*thenode2);

                // See if a recur link already exists  ALSO STOP AT END OF GENES!!!!
                thegene = genes.begin();
                while ((thegene != genes.end()) &&
                       ((nodep2->type) != SENSOR) && // Don't allow SENSORS to get input
                       (!((((*thegene)->link)->in_node == nodep1) &&
                          (((*thegene)->link)->out_node == nodep2) &&
                          ((*thegene)->link)->is_recurrent)))
                {
                    ++thegene;
                }

                if (thegene != genes.end())
                    trycount++;
                else
                {
                    count = 0;
                    recurflag = phenotype->is_recur(nodep1->analogue, nodep2->analogue, count, thresh);

                    // ADDED: CONSIDER connections out of outputs recurrent
                    if (((nodep1->gen_node_label) == OUTPUT) ||
                        ((nodep2->gen_node_label) == OUTPUT))
                        recurflag = true;

                    // Exit if the network is faulty (contains an infinite loop)
                    // NOTE: A loop doesn't really matter
                    // if (count>thresh) {
                    //   cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<std::endl;
                    //   return false;
                    // }

                    // Make sure it finds the right kind of link (recur)
                    if (!(recurflag))
                        trycount++;
                    else
                    {
                        trycount = tries;
                        found = true;
                    }
                }
            }
        }
        else
        {
            // Loop to find a nonrecurrent link
            while (trycount < tries)
            {
                // cout<<"TRY "<<trycount<<std::endl;

                // Choose random nodenums
                nodenum1 = neat.randint(0, nodes.size() - 1);
                nodenum2 = neat.randint(first_nonsensor, nodes.size() - 1);

                // Find the first node
                thenode1 = nodes.begin();
                for (nodecount = 0; nodecount < nodenum1; nodecount++)
                    ++thenode1;

                // cout<<"RETRIEVED NODE# "<<(*thenode1)->node_id<<std::endl;

                // Find the second node
                thenode2 = nodes.begin();
                for (nodecount = 0; nodecount < nodenum2; nodecount++)
                    ++thenode2;

                nodep1 = (*thenode1);
                nodep2 = (*thenode2);

                // See if a link already exists  ALSO STOP AT END OF GENES!!!!
                thegene = genes.begin();
                while ((thegene != genes.end()) &&
                       ((nodep2->type) != SENSOR) && // Don't allow SENSORS to get input
                       (!((((*thegene)->link)->in_node == nodep1) &&
                          (((*thegene)->link)->out_node == nodep2) &&
                          (!(((*thegene)->link)->is_recurrent)))))
                {
                    ++thegene;
                }

                if (thegene != genes.end())
                    trycount++;
                else
                {
                    count = 0;
                    recurflag = phenotype->is_recur(nodep1->analogue, nodep2->analogue, count, thresh);

                    // ADDED: CONSIDER connections out of outputs recurrent
                    if (((nodep1->gen_node_label) == OUTPUT) ||
                        ((nodep2->gen_node_label) == OUTPUT))
                        recurflag = true;

                    // Exit if the network is faulty (contains an infinite loop)
                    if (count > thresh)
                    {
                        // cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<std::endl;
                        // return false;
                    }

                    // Make sure it finds the right kind of link (recur or not)
                    if (recurflag)
                        trycount++;
                    else
                    {
                        trycount = tries;
                        found = true;
                    }
                }

            } // End of normal link finding loop
        }

        // Continue only if an open link was found
        if (found)
        {
            // Check to see if this innovation already occured in the population
            theinnov = innovs.begin();

            // If it was supposed to be recurrent, make sure it gets labeled that way
            if (do_recur)
                recurflag = 1;

            done = false;

            while (!done)
            {
                // The innovation is totally novel
                if (theinnov == innovs.end())
                {
                    // If the phenotype does not exist, exit on false,print error
                    // Note: This should never happen- if it does there is a bug
                    if (phenotype)
                    {
                        // cout<<"ERROR: Attempt to add link to genome with no phenotype"<<std::endl;
                        return false;
                    }

                    // Useful for debugging
                    // cout<<"nodep1 id: "<<nodep1->node_id<<std::endl;
                    // cout<<"nodep1: "<<nodep1<<std::endl;
                    // cout<<"nodep1 analogue: "<<nodep1->analogue<<std::endl;
                    // cout<<"nodep2 id: "<<nodep2->node_id<<std::endl;
                    // cout<<"nodep2: "<<nodep2<<std::endl;
                    // cout<<"nodep2 analogue: "<<nodep2->analogue<<std::endl;
                    // cout<<"recurflag: "<<recurflag<<std::endl;

                    // NOTE: Something like this could be used for time delays,
                    //       which are not yet supported.  However, this does not
                    //       have an application with recurrency.
                    // If not recurrent, randomize recurrency
                    // if (!recurflag)
                    //   if (randfloat()<recur_prob) recurflag=1;

                    // Choose a random trait
                    traitnum = neat.randint(0, (traits.size()) - 1);
                    thetrait = traits.begin();

                    // Choose the new weight
                    // newweight=(gaussrand())/1.5;  //Could use a gaussian
                    newweight = neat.randposneg() * neat.randfloat() * 1.0; // used to be 10.0

                    // Create the new gene
                    newgene = Gene::makeFromTrait(neat, thetrait[traitnum], newweight, nodep1, nodep2, recurflag, curinnov, newweight);

                    // Add the innovation
                    auto theInnov = Innovation::makeAsNewLinkType(nodep1->node_id, nodep2->node_id, curinnov, newweight, traitnum);
                    innovs.push_back(theInnov);

                    curinnov += 1.0;

                    done = true;
                }
                // OTHERWISE, match the innovation in the innovs list
                else if (((*theinnov)->innovation_type == NEWLINK) &&
                         ((*theinnov)->node_in_id == (nodep1->node_id)) &&
                         ((*theinnov)->node_out_id == (nodep2->node_id)) &&
                         ((*theinnov)->recur_flag == (bool)recurflag))
                {
                    thetrait = traits.begin();

                    // Create new gene
                    newgene = Gene::makeFromTrait(neat, thetrait[(*theinnov)->new_traitnum], (*theinnov)->new_weight, nodep1, nodep2, recurflag, (*theinnov)->innovation_num1, 0);

                    done = true;
                }
                else
                {
                    // Keep looking for a matching innovation from this generation
                    ++theinnov;
                }
            }

            // Now add the new Genes to the Genome
            // genes.push_back(newgene);  //Old way - could result in out-of-order innovation numbers in rtNEAT
            add_gene(genes, newgene); // Adds the gene in correct order

            return true;
        }
        else
        {
            return false;
        }
    }

    void Genome::mutate_add_sensor(const Neat &neat,
                                   std::vector<std::shared_ptr<Innovation>> &innovs,
                                   double &curinnov)
    {

        std::vector<std::shared_ptr<NNode>> sensors;
        std::vector<std::shared_ptr<NNode>> outputs;
        std::shared_ptr<NNode> node;
        std::shared_ptr<NNode> sensor;
        std::shared_ptr<NNode> output;
        std::shared_ptr<Gene> gene;

        double newweight = 0.0;
        std::shared_ptr<Gene> newgene;

        int i, j; // counters
        bool found;

        bool done;

        int outputConnections;

        std::vector<std::shared_ptr<Trait>>::iterator thetrait;
        int traitnum;

        std::vector<std::shared_ptr<Innovation>>::iterator theinnov; // For finding a historical match

        // Find all the sensors and outputs
        for (i = 0; i < nodes.size(); i++)
        {
            node = nodes[i];

            if ((node->type) == SENSOR)
                sensors.push_back(node);
            else if (node->gen_node_label == OUTPUT)
                outputs.push_back(node);
        }

        // eliminate from contention any sensors that are already connected
        sensors.erase(std::remove_if(sensors.begin(), sensors.end(),
                                     [&](const std::shared_ptr<NNode> &sensor)
                                     {
                                         int outputConnections = 0;
                                         for (const auto &gene : genes)
                                         {
                                             if (gene->link->out_node->gen_node_label == OUTPUT)
                                             {
                                                 outputConnections++;
                                             }
                                         }
                                         return outputConnections == outputs.size();
                                     }),
                      sensors.end());

        // If all sensors are connected, quit
        if (sensors.empty())
            return;

        // Pick randomly from remaining sensors
        sensor = sensors[neat.randint(0, sensors.size() - 1)];

        // Add new links to chosen sensor, avoiding redundancy
        for (int i = 0; i < outputs.size(); i++)
        {
            output = outputs[i];

            found = false;
            for (j = 0; j < genes.size(); j++)
            {
                gene = genes[j];
                if ((gene->link->in_node == sensor) &&
                    (gene->link->out_node == output))
                    found = true;
            }

            // Record the innovation
            if (!found)
            {
                theinnov = innovs.begin();
                done = false;

                while (!done)
                {
                    // The innovation is novel
                    if (theinnov == innovs.end())
                    {

                        // Choose a random trait
                        traitnum = neat.randint(0, (traits.size()) - 1);
                        thetrait = traits.begin();

                        // Choose the new weight
                        // newweight=(gaussrand())/1.5;  //Could use a gaussian
                        newweight = neat.randposneg() * neat.randfloat() * 3.0; // used to be 10.0

                        // Create the new gene
                        newgene = Gene::makeFromTrait(neat, thetrait[traitnum], newweight, sensor, output, false, curinnov, newweight);

                        auto newInnov = Innovation::makeAsNewLinkType(sensor->node_id, output->node_id, curinnov, newweight, traitnum);
                        // Add the innovation
                        innovs.push_back(newInnov);

                        curinnov += 1.0;

                        done = true;
                    } // end novel innovation case
                    // OTHERWISE, match the innovation in the innovs list
                    else if (((*theinnov)->innovation_type == NEWLINK) &&
                             ((*theinnov)->node_in_id == (sensor->node_id)) &&
                             ((*theinnov)->node_out_id == (output->node_id)) &&
                             ((*theinnov)->recur_flag == false))
                    {

                        thetrait = traits.begin();

                        // Create new gene
                        newgene = Gene::makeFromTrait(neat,
                                                      thetrait[(*theinnov)->new_traitnum],
                                                      (*theinnov)->new_weight, sensor, output,
                                                      false, (*theinnov)->innovation_num1, 0);

                        done = true;
                    } // end prior innovation case
                    // Keep looking for matching innovation
                    else
                        ++theinnov;

                } // end while

                add_gene(genes, newgene); // adds the gene in correct order

            } // end case where the gene didn't previously exist
        }
    }

    std::shared_ptr<Genome> Genome::mate_multipoint(
        const Neat &neat,
        std::shared_ptr<Genome> g,
        int genomeid,
        double fitness1, double fitness2, bool interspec_flag)
    {
        // The baby Genome will contain these new Traits, NNodes, and Genes
        std::vector<std::shared_ptr<Trait>> newtraits;
        std::vector<std::shared_ptr<NNode>> newnodes;
        std::vector<std::shared_ptr<Gene>> newgenes;

        // iterators for moving through the two parents' genes
        auto p1gene = genes.begin();
        auto p2gene = (g->genes).begin();
        double p1innov; // Innovation numbers for genes inside parents' Genomes
        double p2innov;
        std::shared_ptr<Gene> chosengene; // Gene chosen for baby to inherit
        int traitnum;                     // Number of trait new gene points to
        std::shared_ptr<NNode> inode;     // NNodes connected to the chosen Gene
        std::shared_ptr<NNode> onode;
        std::shared_ptr<NNode> new_inode;
        std::shared_ptr<NNode> new_onode;

        bool disable; // Set to true if we want to disabled a chosen gene

        disable = false;
        std::shared_ptr<Gene> newgene;

        bool p1better; // Tells if the first genome (this one) has better fitness or not

        bool skip;

        // First, average the Traits from the 2 parents to form the baby's Traits
        // It is assumed that trait lists are the same length
        // In the future, may decide on a different method for trait mating
        for (size_t i = 0; i < traits.size(); ++i)
        {
            auto newtrait = Trait::makeByAverage(neat, *traits[i], *(g->traits[i]));
            newtraits.push_back(newtrait);
        }

        // Figure out which genome is better
        // The worse genome should not be allowed to add extra structural baggage
        // If they are the same, use the smaller one's disjoint and excess genes only
        if (fitness1 == fitness2)
        {
            if (genes.size() < (g->genes.size()))
                p1better = true;
            else
                p1better = false;
        }
        else
        {
            p1better = fitness1 > fitness2;
        }

        // NEW 3/17/03 Make sure all sensors and outputs are included
        for (const auto &curnode : g->nodes)
        {
            if ((curnode->gen_node_label == INPUT) ||
                (curnode->gen_node_label == BIAS) ||
                (curnode->gen_node_label == OUTPUT))
            {
                int nodetraitnum;
                if (!curnode->nodetrait)
                    nodetraitnum = 0;
                else
                    nodetraitnum = curnode->nodetrait->trait_id - traits.front()->trait_id;

                // Create a new node off the sensor or output
                new_onode = NNode::makeFromTrait(*curnode, newtraits[nodetraitnum]);

                // Add the new node
                node_insert(newnodes, new_onode);
            }
        }

        // Now move through the Genes of each parent until both genomes end
        while (!((p1gene == genes.end()) &&
                 (p2gene == (g->genes).end())))
        {
            skip = false; // Default to not skipping a chosen gene

            if (p1gene == genes.end())
            {
                chosengene = *p2gene;
                ++p2gene;
                if (p1better)
                    skip = true; // Skip excess from the worse genome
            }
            else if (p2gene == (g->genes).end())
            {
                chosengene = *p1gene;
                ++p1gene;
                if (!p1better)
                    skip = true; // Skip excess from the worse genome
            }
            else
            {
                // Extract current innovation numbers
                p1innov = (*p1gene)->innovation_num;
                p2innov = (*p2gene)->innovation_num;

                if (p1innov == p2innov)
                {
                    if (neat.randfloat() < 0.5)
                    {
                        chosengene = *p1gene;
                    }
                    else
                    {
                        chosengene = *p2gene;
                    }

                    // If one is disabled, the corresponding gene in the offspring
                    // will likely be disabled
                    if ((((*p1gene)->enable) == false) ||
                        (((*p2gene)->enable) == false))
                        if (neat.randfloat() < 0.75)
                            disable = true;

                    ++p1gene;
                    ++p2gene;
                }
                else if (p1innov < p2innov)
                {
                    chosengene = *p1gene;
                    ++p1gene;

                    if (!p1better)
                        skip = true;
                }
                else if (p2innov < p1innov)
                {
                    chosengene = *p2gene;
                    ++p2gene;
                    if (p1better)
                        skip = true;
                }
            }

            /*
            //Uncomment this line to let growth go faster (from both parents excesses)
            skip=false;

            //For interspecies mating, allow all genes through:
            if (interspec_flag)
                skip=false;
            */

            // Check to see if the chosengene conflicts with an already chosen gene
            // i.e. do they represent the same link
            auto curgene2 = std::find_if(newgenes.begin(), newgenes.end(),
                                         [&](const std::shared_ptr<Gene> &g)
                                         {
                                             return (g->link->in_node->node_id == chosengene->link->in_node->node_id &&
                                                     g->link->out_node->node_id == chosengene->link->out_node->node_id &&
                                                     g->link->is_recurrent == chosengene->link->is_recurrent) ||
                                                    (g->link->in_node->node_id == chosengene->link->out_node->node_id &&
                                                     g->link->out_node->node_id == chosengene->link->in_node->node_id &&
                                                     !g->link->is_recurrent &&
                                                     !chosengene->link->is_recurrent);
                                         });

            if (curgene2 != newgenes.end())
                skip = true; // Links conflicts, abort adding

            if (!skip)
            {
                // Now add the chosengene to the baby

                // First, get the trait pointer
                if (!chosengene->link->linktrait)
                    traitnum = traits.front()->trait_id - 1;
                else
                    // The subtracted number normalizes depending on whether traits start counting at 1 or 0
                    traitnum = chosengene->link->linktrait->trait_id - traits.front()->trait_id;

                // Next check for the nodes, add them if not in the baby Genome already
                inode = (chosengene->link)->in_node;
                onode = (chosengene->link)->out_node;

                // Check for inode in the newnodes list
                if (inode->node_id < onode->node_id)
                {
                    // inode before onode

                    // Checking for inode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == inode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        //(normalized trait number for new NNode)

                        // old buggy version:
                        //  if (!(onode->nodetrait)) nodetraitnum=((*(traits.begin()))->trait_id);
                        int nodetraitnum;
                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = inode->nodetrait->trait_id - traits.front()->trait_id;

                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);
                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                    // Checking for onode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == onode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        int nodetraitnum;

                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = onode->nodetrait->trait_id - traits.front()->trait_id;

                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);
                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }
                }
                // If the onode has a higher id than the inode we want to add it first
                else
                {
                    // Checking for onode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == onode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        int nodetraitnum;

                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((onode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;

                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);
                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }

                    // Checking for inode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == inode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        int nodetraitnum;

                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((inode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;

                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);
                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                } // End NNode checking section- NNodes are now in new Genome

                // Add the Gene
                newgene = Gene::makeFromGene(neat, *chosengene, newtraits[traitnum], new_inode, new_onode);
                if (disable)
                {
                    newgene->enable = false;
                    disable = false;
                }
                newgenes.push_back(newgene);
            }
        }

        auto new_genome = Genome::makeFromSpecs(genomeid, newtraits, newnodes, newgenes);

        // Return the baby Genome
        return new_genome;
    }

    std::shared_ptr<Genome> Genome::mate_multipoint_avg(
        const Neat &neat,
        std::shared_ptr<Genome> g,
        int genomeid,
        double fitness1, double fitness2,
        bool interspec_flag)
    {
        // The baby Genome will contain these new Traits, NNodes, and Genes
        std::vector<std::shared_ptr<Trait>> newtraits;
        std::vector<std::shared_ptr<NNode>> newnodes;
        std::vector<std::shared_ptr<Gene>> newgenes;

        // iterators for moving through the two parents' traits
        std::vector<std::shared_ptr<Trait>>::iterator p1trait;
        std::vector<std::shared_ptr<Trait>>::iterator p2trait;
        std::shared_ptr<Trait> newtrait;

        std::vector<std::shared_ptr<Gene>>::iterator curgene2; // Checking for link duplication

        // iterators for moving through the two parents' genes
        std::vector<std::shared_ptr<Gene>>::iterator p1gene;
        std::vector<std::shared_ptr<Gene>>::iterator p2gene;
        double p1innov; // Innovation numbers for genes inside parents' Genomes
        double p2innov;
        std::shared_ptr<Gene> chosengene; // Gene chosen for baby to inherit
        int traitnum;                     // Number of trait new gene points to
        std::shared_ptr<NNode> inode;     // NNodes connected to the chosen Gene
        std::shared_ptr<NNode> onode;
        std::shared_ptr<NNode> new_inode;
        std::shared_ptr<NNode> new_onode;

        std::vector<std::shared_ptr<NNode>>::iterator curnode; // For checking if NNodes exist already
        int nodetraitnum;                                      // Trait number for a NNode

        // This Gene is used to hold the average of the two genes to be averaged
        std::shared_ptr<Gene> avgene;

        std::shared_ptr<Gene> newgene;

        bool skip;

        bool p1better; // Designate the better genome

        // BLX-alpha variables - for assigning weights within a good space
        // This is for BLX-style mating, which isn't used in this implementation,
        //   but can easily be made from multipoint_avg
        // double blx_alpha;
        // double w1,w2;
        // double blx_min, blx_max;
        // double blx_range;   //The space range
        // double blx_explore;  //Exploration space on left or right
        // double blx_pos;  //Decide where to put gnes distancewise
        // blx_pos=randfloat();

        // First, average the Traits from the 2 parents to form the baby's Traits
        // It is assumed that trait lists are the same length
        // In future, could be done differently
        for (size_t i = 0; i < traits.size(); ++i)
        {
            auto newtrait = Trait::makeByAverage(neat, *traits[i], *(g->traits[i]));
            newtraits.push_back(newtrait);
        }

        // Set up the avgene
        avgene = Gene::makeFromTrait(neat, nullptr, 0, nullptr, nullptr, 0, 0, 0);

        // NEW 3/17/03 Make sure all sensors and outputs are included
        for (const auto &curnode : g->nodes)
        {
            if ((curnode->gen_node_label == INPUT) ||
                (curnode->gen_node_label == OUTPUT) ||
                (curnode->gen_node_label == BIAS))
            {
                if (!curnode->nodetrait)
                    nodetraitnum = 0;
                else
                    nodetraitnum = curnode->nodetrait->trait_id - traits.front()->trait_id;

                // Create a new node off the sensor or output
                new_onode = NNode::makeFromTrait(*curnode, newtraits[nodetraitnum]);

                // Add the new node
                node_insert(newnodes, new_onode);
            }
        }

        // Figure out which genome is better
        // The worse genome should not be allowed to add extra structural baggage
        // If they are the same, use the smaller one's disjoint and excess genes only
        if (fitness1 > fitness2)
            p1better = true;
        else if (fitness1 == fitness2)
        {
            if (genes.size() < (g->genes.size()))
                p1better = true;
            else
                p1better = false;
        }
        else
            p1better = false;

        // Now move through the Genes of each parent until both genomes end
        p1gene = genes.begin();
        p2gene = (g->genes).begin();
        while (!((p1gene == genes.end()) &&
                 (p2gene == (g->genes).end())))
        {
            avgene->enable = true; // Default to enabled

            skip = false;

            if (p1gene == genes.end())
            {
                chosengene = *p2gene;
                ++p2gene;

                if (p1better)
                    skip = true;
            }
            else if (p2gene == (g->genes).end())
            {
                chosengene = *p1gene;
                ++p1gene;

                if (!p1better)
                    skip = true;
            }
            else
            {
                // Extract current innovation numbers
                p1innov = (*p1gene)->innovation_num;
                p2innov = (*p2gene)->innovation_num;

                if (p1innov == p2innov)
                {
                    // Average them into the avgene
                    if (neat.randfloat() > 0.5)
                        (avgene->link)->linktrait = ((*p1gene)->link)->linktrait;
                    else
                        (avgene->link)->linktrait = ((*p2gene)->link)->linktrait;

                    // WEIGHTS AVERAGED HERE
                    (avgene->link)->weight = (((*p1gene)->link)->weight + ((*p2gene)->link)->weight) / 2.0;

                    ////BLX-alpha method (Eschelman et al 1993)
                    ////Not used in this implementation, but the commented code works
                    ////with alpha=0.5, this will produce babies evenly in exploitation and exploration space
                    ////and uniformly distributed throughout
                    // blx_alpha=-0.4;
                    // w1=(((*p1gene)->lnk)->weight);
                    // w2=(((*p2gene)->lnk)->weight);
                    // if (w1>w2) {
                    // blx_max=w1; blx_min=w2;
                    // }
                    // else {
                    // blx_max=w2; blx_min=w1;
                    // }
                    // blx_range=blx_max-blx_min;
                    // blx_explore=blx_alpha*blx_range;
                    ////Now extend the range into the exploraton space
                    // blx_min-=blx_explore;
                    // blx_max+=blx_explore;
                    // blx_range=blx_max-blx_min;
                    ////Set the weight in the new range
                    //(avgene->lnk)->weight=blx_min+blx_pos*blx_range;
                    //

                    if (neat.randfloat() > 0.5)
                        (avgene->link)->in_node = ((*p1gene)->link)->in_node;
                    else
                        (avgene->link)->in_node = ((*p2gene)->link)->in_node;

                    if (neat.randfloat() > 0.5)
                        (avgene->link)->out_node = ((*p1gene)->link)->out_node;
                    else
                        (avgene->link)->out_node = ((*p2gene)->link)->out_node;

                    if (neat.randfloat() > 0.5)
                        (avgene->link)->is_recurrent = ((*p1gene)->link)->is_recurrent;
                    else
                        (avgene->link)->is_recurrent = ((*p2gene)->link)->is_recurrent;

                    avgene->innovation_num = (*p1gene)->innovation_num;
                    avgene->mutation_num = ((*p1gene)->mutation_num + (*p2gene)->mutation_num) / 2.0;

                    if ((((*p1gene)->enable) == false) ||
                        (((*p2gene)->enable) == false))
                        if (neat.randfloat() < 0.75)
                            avgene->enable = false;

                    chosengene = avgene;
                    ++p1gene;
                    ++p2gene;
                }
                else if (p1innov < p2innov)
                {
                    chosengene = *p1gene;
                    ++p1gene;

                    if (!p1better)
                        skip = true;
                }
                else if (p2innov < p1innov)
                {
                    chosengene = *p2gene;
                    ++p2gene;

                    if (p1better)
                        skip = true;
                }
            }

            /*
            //THIS LINE MUST BE DELETED TO SLOW GROWTH
            skip=false;

            //For interspecies mating, allow all genes through:
            if (interspec_flag)
                skip=false;
            */

            // Check to see if the chosengene conflicts with an already chosen gene
            // i.e. do they represent the same link
            curgene2 = newgenes.begin();
            while ((curgene2 != newgenes.end()))

            {
                if (((((((*curgene2)->link)->in_node)->node_id) == ((((chosengene)->link)->in_node)->node_id)) &&
                     (((((*curgene2)->link)->out_node)->node_id) == ((((chosengene)->link)->out_node)->node_id)) &&
                     ((((*curgene2)->link)->is_recurrent) == (((chosengene)->link)->is_recurrent))) ||
                    ((((((*curgene2)->link)->out_node)->node_id) == ((((chosengene)->link)->in_node)->node_id)) &&
                     (((((*curgene2)->link)->in_node)->node_id) == ((((chosengene)->link)->out_node)->node_id)) &&
                     (!((((*curgene2)->link)->is_recurrent))) &&
                     (!((((chosengene)->link)->is_recurrent)))))
                {
                    skip = true;
                }
                ++curgene2;
            }

            if (!skip)
            {

                // Now add the chosengene to the baby

                // First, get the trait pointer
                if ((((chosengene->link)->linktrait)) == 0)
                    traitnum = (*(traits.begin()))->trait_id - 1;
                else
                    // The subtracted number normalizes depending on whether traits start counting at 1 or 0
                    traitnum = (((chosengene->link)->linktrait)->trait_id) - (*(traits.begin()))->trait_id;

                // Next check for the nodes, add them if not in the baby Genome already
                inode = (chosengene->link)->in_node;
                onode = (chosengene->link)->out_node;

                // Check for inode in the newnodes list
                if (inode->node_id < onode->node_id)
                {

                    // Checking for inode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == inode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode

                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((inode->nodetrait)->trait_id) - ((*(traits.begin()))->trait_id);

                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                    // Checking for onode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == onode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        int nodetraitnum;
                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = onode->nodetrait->trait_id - traits.front()->trait_id;

                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }
                }
                // If the onode has a higher id than the inode we want to add it first
                else
                {
                    // Checking for onode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == onode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((onode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;

                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }

                    // Checking for inode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == inode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        int nodetraitnum;
                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = inode->nodetrait->trait_id - traits.front()->trait_id;

                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                } // End NNode checking section- NNodes are now in new Genome

                // Add the Gene
                newgene = Gene::makeFromGene(neat, *chosengene, newtraits[traitnum], new_inode, new_onode);

                newgenes.push_back(newgene);

            } // End if which checked for link duplicationb
        }

        // Return the baby Genome
        auto newGenome = Genome::makeFromSpecs(genomeid, newtraits, newnodes, newgenes);

        return (newGenome);
    }

    std::shared_ptr<Genome> Genome::mate_singlepoint(
        const Neat &neat,
        std::shared_ptr<Genome> g,
        int genomeid)
    {
        // The baby Genome will contain these new Traits, NNodes, and Genes
        std::vector<std::shared_ptr<Trait>> newtraits;
        std::vector<std::shared_ptr<NNode>> newnodes;
        std::vector<std::shared_ptr<Gene>> newgenes;

        // iterators for moving through the two parents' genes
        std::vector<std::shared_ptr<Gene>>::iterator p1gene;
        std::vector<std::shared_ptr<Gene>>::iterator p2gene;
        std::vector<std::shared_ptr<Gene>>::iterator stopper; // To tell when finished
        std::vector<std::shared_ptr<Gene>>::iterator p2stop;
        std::vector<std::shared_ptr<Gene>>::iterator p1stop;
        double p1innov; // Innovation numbers for genes inside parents' Genomes
        double p2innov;
        std::shared_ptr<Gene> chosengene; // Gene chosen for baby to inherit
        int traitnum;                     // Number of trait new gene points to
        std::shared_ptr<NNode> inode;     // NNodes connected to the chosen Gene
        std::shared_ptr<NNode> onode;
        std::shared_ptr<NNode> new_inode;
        std::shared_ptr<NNode> new_onode;
        // curnode; For checking if NNodes exist already
        int nodetraitnum; // Trait number for a NNode

        int crosspoint;  // The point in the Genome to cross at
        int genecounter; // Counts up to the crosspoint
        bool skip;       // Used for skipping unwanted genes

        // First, average the Traits from the 2 parents to form the baby's Traits
        // It is assumed that trait lists are the same length
        for (size_t i = 0; i < traits.size(); ++i)
        {
            auto newtrait = Trait::makeByAverage(neat, *traits[i], *(g->traits[i])); // Construct by averaging
            newtraits.push_back(newtrait);
        }

        // Set up the avgene
        // This Gene is used to hold the average of the two genes to be averaged
        auto avgene = Gene::makeFromTrait(neat, nullptr, 0, nullptr, nullptr, 0, 0, 0);

        // Decide where to cross  (p1gene will always be in smaller Genome)
        if (genes.size() < (g->genes).size())
        {
            crosspoint = neat.randint(0, (genes.size()) - 1);
            p1gene = genes.begin();
            p2gene = (g->genes).begin();
            stopper = (g->genes).end();
            p1stop = genes.end();
            p2stop = (g->genes).end();
        }
        else
        {
            crosspoint = neat.randint(0, ((g->genes).size()) - 1);
            p2gene = genes.begin();
            p1gene = (g->genes).begin();
            stopper = genes.end();
            p1stop = (g->genes.end());
            p2stop = genes.end();
        }

        genecounter = 0; // Ready to count to crosspoint

        skip = false; // Default to not skip a Gene
        // Note that we skip when we are on the wrong Genome before
        // crossing

        // Now move through the Genes of each parent until both genomes end
        while (p2gene != stopper)
        {
            avgene->enable = true; // Default to true

            if (p1gene == p1stop)
            {
                chosengene = *p2gene;
                ++p2gene;
            }
            else if (p2gene == p2stop)
            {
                chosengene = *p1gene;
                ++p1gene;
            }
            else
            {
                // Extract current innovation numbers

                // if (p1gene==g->genes.end()) cout<<"WARNING p1"<<std::endl;
                // if (p2gene==g->genes.end()) cout<<"WARNING p2"<<std::endl;

                p1innov = (*p1gene)->innovation_num;
                p2innov = (*p2gene)->innovation_num;

                if (p1innov == p2innov)
                {

                    // Pick the chosengene depending on whether we've crossed yet
                    if (genecounter < crosspoint)
                    {
                        chosengene = *p1gene;
                    }
                    else if (genecounter > crosspoint)
                    {
                        chosengene = *p2gene;
                    }
                    // We are at the crosspoint here
                    else
                    {

                        // Average them into the avgene
                        if (neat.randfloat() > 0.5)
                            (avgene->link)->linktrait = ((*p1gene)->link)->linktrait;
                        else
                            (avgene->link)->linktrait = ((*p2gene)->link)->linktrait;

                        // WEIGHTS AVERAGED HERE
                        (avgene->link)->weight = (((*p1gene)->link)->weight + ((*p2gene)->link)->weight) / 2.0;

                        if (neat.randfloat() > 0.5)
                            (avgene->link)->in_node = ((*p1gene)->link)->in_node;
                        else
                            (avgene->link)->in_node = ((*p2gene)->link)->in_node;

                        if (neat.randfloat() > 0.5)
                            (avgene->link)->out_node = ((*p1gene)->link)->out_node;
                        else
                            (avgene->link)->out_node = ((*p2gene)->link)->out_node;

                        if (neat.randfloat() > 0.5)
                            (avgene->link)->is_recurrent = ((*p1gene)->link)->is_recurrent;
                        else
                            (avgene->link)->is_recurrent = ((*p2gene)->link)->is_recurrent;

                        avgene->innovation_num = (*p1gene)->innovation_num;
                        avgene->mutation_num = ((*p1gene)->mutation_num + (*p2gene)->mutation_num) / 2.0;

                        if ((((*p1gene)->enable) == false) ||
                            (((*p2gene)->enable) == false))
                            avgene->enable = false;

                        chosengene = avgene;
                    }

                    ++p1gene;
                    ++p2gene;
                    ++genecounter;
                }
                else if (p1innov < p2innov)
                {
                    if (genecounter < crosspoint)
                    {
                        chosengene = *p1gene;
                        ++p1gene;
                        ++genecounter;
                    }
                    else
                    {
                        chosengene = *p2gene;
                        ++p2gene;
                    }
                }
                else if (p2innov < p1innov)
                {
                    ++p2gene;
                    skip = true; // Special case: we need to skip to the next iteration
                                 // becase this Gene is before the crosspoint on the wrong Genome
                }
            }

            // Check to see if the chosengene conflicts with an already chosen gene
            // i.e. do they represent the same link
            auto curgene2 = newgenes.begin();

            while ((curgene2 != newgenes.end()) &&
                   (!((((((*curgene2)->link)->in_node)->node_id) == ((((chosengene)->link)->in_node)->node_id)) &&
                      (((((*curgene2)->link)->out_node)->node_id) == ((((chosengene)->link)->out_node)->node_id)) &&
                      ((((*curgene2)->link)->is_recurrent) == (((chosengene)->link)->is_recurrent)))) &&
                   (!((((((*curgene2)->link)->in_node)->node_id) == ((((chosengene)->link)->out_node)->node_id)) &&
                      (((((*curgene2)->link)->out_node)->node_id) == ((((chosengene)->link)->in_node)->node_id)) &&
                      (!((((*curgene2)->link)->is_recurrent))) &&
                      (!((((chosengene)->link)->is_recurrent))))))
            {
                ++curgene2;
            }

            if (curgene2 != newgenes.end())
                skip = true; // Link is a duplicate

            if (!skip)
            {
                // Now add the chosengene to the baby

                // First, get the trait pointer
                if ((((chosengene->link)->linktrait)) == 0)
                    traitnum = (*(traits.begin()))->trait_id - 1;
                else
                    traitnum = (((chosengene->link)->linktrait)->trait_id) - (*(traits.begin()))->trait_id; // The subtracted number normalizes depending on whether traits start counting at 1 or 0

                // Next check for the nodes, add them if not in the baby Genome already
                inode = (chosengene->link)->in_node;
                onode = (chosengene->link)->out_node;

                // Check for inode in the newnodes list
                if (inode->node_id < onode->node_id)
                {
                    // cout<<"inode before onode"<<std::endl;
                    // Checking for inode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == inode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode

                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((inode->nodetrait)->trait_id) - ((*(traits.begin()))->trait_id);

                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                    // Checking for onode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == onode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode

                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((onode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;
                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }
                }
                // If the onode has a higher id than the inode we want to add it first
                else
                {
                    // Checking for onode's existence
                    auto curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                                [&](const std::shared_ptr<NNode> &n)
                                                {
                                                    return n->node_id == onode->node_id;
                                                });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        if (!(onode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((onode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;
                        new_onode = NNode::makeFromTrait(*onode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_onode);
                    }
                    else
                    {
                        new_onode = (*curnode);
                    }

                    // Checking for inode's existence
                    curnode = std::find_if(newnodes.begin(), newnodes.end(),
                                           [&](const std::shared_ptr<NNode> &n)
                                           {
                                               return n->node_id == inode->node_id;
                                           });
                    if (curnode == newnodes.end())
                    {
                        // Here we know the node doesn't exist so we have to add it
                        // normalized trait number for new NNode
                        if (!(inode->nodetrait))
                            nodetraitnum = 0;
                        else
                            nodetraitnum = ((inode->nodetrait)->trait_id) - (*(traits.begin()))->trait_id;
                        new_inode = NNode::makeFromTrait(*inode, newtraits[nodetraitnum]);

                        node_insert(newnodes, new_inode);
                    }
                    else
                    {
                        new_inode = (*curnode);
                    }

                } // End NNode checking section- NNodes are now in new Genome

                // Add the Gene
                auto newGene = Gene::makeFromGene(neat, *chosengene, newtraits[traitnum], new_inode, new_onode);
                newgenes.push_back(newGene);

            } // End of if (!skip)

            skip = false;
        }

        // Return the baby Genome
        auto newGenome = Genome::makeFromSpecs(genomeid, newtraits, newnodes, newgenes);
        return newGenome;
    }

    double Genome::compatibility(const Neat &neat,
                                 const std::shared_ptr<Genome> g)
    {
        // iterators for moving through the two potential parents' Genes
        std::vector<std::shared_ptr<Gene>>::iterator p1gene;
        std::vector<std::shared_ptr<Gene>>::iterator p2gene;

        // Innovation numbers
        double p1innov;
        double p2innov;

        // Intermediate value
        double mut_diff;

        // Set up the counters
        double num_disjoint = 0.0;
        double num_excess = 0.0;
        double mut_diff_total = 0.0;
        double num_matching = 0.0; // Used to normalize mutation_num differences

        double max_genome_size; // Size of larger Genome

        // Get the length of the longest Genome for percentage computations
        if (genes.size() < (g->genes).size())
            max_genome_size = (g->genes).size();
        else
            max_genome_size = genes.size();

        // Now move through the Genes of each potential parent
        // until both Genomes end
        p1gene = genes.begin();
        p2gene = (g->genes).begin();
        while (!((p1gene == genes.end()) &&
                 (p2gene == (g->genes).end())))
        {

            if (p1gene == genes.end())
            {
                ++p2gene;
                num_excess += 1.0;
            }
            else if (p2gene == (g->genes).end())
            {
                ++p1gene;
                num_excess += 1.0;
            }
            else
            {
                // Extract current innovation numbers
                p1innov = (*p1gene)->innovation_num;
                p2innov = (*p2gene)->innovation_num;

                if (p1innov == p2innov)
                {
                    num_matching += 1.0;
                    mut_diff = ((*p1gene)->mutation_num) - ((*p2gene)->mutation_num);
                    if (mut_diff < 0.0)
                        mut_diff = 0.0 - mut_diff;
                    // mut_diff+=trait_compare((*p1gene)->lnk->linktrait,(*p2gene)->lnk->linktrait); //CONSIDER TRAIT DIFFERENCES
                    mut_diff_total += mut_diff;

                    ++p1gene;
                    ++p2gene;
                }
                else if (p1innov < p2innov)
                {
                    ++p1gene;
                    num_disjoint += 1.0;
                }
                else if (p2innov < p1innov)
                {
                    ++p2gene;
                    num_disjoint += 1.0;
                }
            }
        } // End while

        // Return the compatibility number using compatibility formula
        // Note that mut_diff_total/num_matching gives the AVERAGE
        // difference between mutation_nums for any two matching Genes
        // in the Genome

        // Normalizing for genome size
        // return (disjoint_coeff*(num_disjoint/max_genome_size)+
        //   excess_coeff*(num_excess/max_genome_size)+
        //   mutdiff_coeff*(mut_diff_total/num_matching));

        // Look at disjointedness and excess in the absolute (ignoring size)

        // cout<<"COMPAT: size = "<<max_genome_size<<" disjoint = "<<num_disjoint<<" excess = "<<num_excess<<" diff = "<<mut_diff_total<<"  TOTAL = "<<(disjoint_coeff*(num_disjoint/1.0)+excess_coeff*(num_excess/1.0)+mutdiff_coeff*(mut_diff_total/num_matching))<<std::endl;

        return (neat.disjoint_coeff * (num_disjoint / 1.0) +
                neat.excess_coeff * (num_excess / 1.0) +
                neat.mutdiff_coeff * (mut_diff_total / num_matching));
    }

    double Genome::trait_compare(const Trait *t1, const Trait *t2)
    {
        int id1 = t1->trait_id;
        int id2 = t2->trait_id;
        int count;
        double params_diff = 0.0; // Measures parameter difference

        // See if traits represent different fundamental types of connections
        if ((id1 == 1) && (id2 >= 2))
        {
            return 0.5;
        }
        else if ((id2 == 1) && (id1 >= 2))
        {
            return 0.5;
        }
        // Otherwise, when types are same, compare the actual parameters
        else
        {
            if (id1 >= 2)
            {
                for (count = 0; count <= 2; count++)
                {
                    params_diff += std::fabs(t1->params[count] - t2->params[count]);
                }
                return params_diff / 4.0;
            }
            else
                return 0.0; // For type 1, params are not applicable
        }
    }

    int Genome::extrons()
    {
        int total = 0;

        for (const auto &curgene : genes)
        {
            if (curgene->enable)
                ++total;
        }

        return total;
    }

    void Genome::randomize_traits(const Neat &neat)
    {
        const int numtraits = traits.size();
        if (numtraits == 0)
        {
            return; // Nothing to do if there are no traits
        }

        // Go through all nodes and randomize their trait pointers
        for (const auto &curnode : nodes)
        {
            int traitnum = neat.randint(1, numtraits); // randomize trait
            curnode->trait_id = traitnum;

            auto curtrait_it = std::find_if(traits.begin(), traits.end(),
                                            [traitnum](const auto &t)
                                            { return t->trait_id == traitnum; });
            if (curtrait_it != traits.end())
            {
                curnode->nodetrait = *curtrait_it;
            }
        }

        // Go through all connections and randomize their trait pointers
        for (const auto &curgene : genes)
        {
            int traitnum = neat.randint(1, numtraits); // randomize trait
            curgene->link->trait_id = traitnum;
            auto curtrait_it = std::find_if(traits.begin(), traits.end(),
                                            [traitnum](const auto &t)
                                            { return t->trait_id == traitnum; });
            if (curtrait_it != traits.end())
            {
                curgene->link->linktrait = *curtrait_it;
            }
        }
    }
} // namespace Neat
