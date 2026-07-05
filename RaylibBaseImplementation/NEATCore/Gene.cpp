#include <fstream>

#include "Gene.h"

namespace Neat
{
    Gene::Gene(/* args */)
    {
    }

    std::shared_ptr<Gene> makeFromNodes(const Neat &neat,
                                        double w,
                                        std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
                                        bool recur, double innov, double mnum)
    {
        auto newGene = std::make_shared<Gene>();

        newGene->link = Link::makeFromNodes(neat, w, inode, onode, recur);
        newGene->innovation_num = innov;
        newGene->mutation_num = mnum;

        newGene->enable = true;

        newGene->frozen = false;

        return newGene;
    }

    std::shared_ptr<Gene> Gene::makeFromTrait(const Neat &neat,
                                              std::shared_ptr<Trait> tp, double w,
                                              std::shared_ptr<NNode> inode,
                                              std::shared_ptr<NNode> onode,
                                              bool recurrent, double innov, double mnum)
    {
        auto newGene = std::make_shared<Gene>();

        newGene->link = Link::makeFromTrait(neat, tp, w, inode, onode, recurrent);
        newGene->innovation_num = innov;
        newGene->mutation_num = mnum;

        newGene->enable = true;

        newGene->frozen = false;

        return newGene;
    }

    std::shared_ptr<Gene> Gene::makeFromTraitNonRecurrent(const Neat &neat,
                                                          std::shared_ptr<Trait> tp, double w,
                                                          std::shared_ptr<NNode> inode,
                                                          std::shared_ptr<NNode> onode,
                                                          double innov, double mnum)
    {
        return makeFromTrait(neat, tp, w, inode, onode, false, innov, mnum);
    }

    std::shared_ptr<Gene> Gene::makeFromTraitRecurrent(const Neat &neat,
                                                       std::shared_ptr<Trait> tp, double w,
                                                       std::shared_ptr<NNode> inode,
                                                       std::shared_ptr<NNode> onode,
                                                       double innov, double mnum)
    {
        return makeFromTrait(neat, tp, w, inode, onode, true, innov, mnum);
    }

    std::shared_ptr<Gene> Gene::makeFromGene(const Neat &neat,
                                             const Gene &g,
                                             std::shared_ptr<Trait> tp,
                                             std::shared_ptr<NNode> inode,
                                             std::shared_ptr<NNode> onode)
    {
        auto newGene = std::make_shared<Gene>();

        // cout<<"Trying to attach nodes: "<<inode<<" "<<onode<<endl;
        newGene->link = Link::makeFromTrait(neat, tp, g.link->weight, inode, onode, g.link->is_recurrent);
        newGene->innovation_num = g.innovation_num;
        newGene->mutation_num = g.mutation_num;
        newGene->enable = g.enable;

        newGene->frozen = g.frozen;

        return newGene;
    }

    std::shared_ptr<Gene> Gene::makeCopy(const Neat &neat, const Gene &gene)
    {
        auto newGene = std::make_shared<Gene>();

        newGene->innovation_num = gene.innovation_num;
        newGene->mutation_num = gene.mutation_num;
        newGene->enable = gene.enable;
        newGene->frozen = gene.frozen;

        newGene->link = Link::makeCopy(neat, gene.link);

        return newGene;
    }

    void Gene::toFile(std::ofstream &outFile)
    {
        outFile << "gene ";
        // Start off with the trait number for this gene
        if ((link->linktrait) == 0)
            outFile << "0 ";
        else
            outFile << ((link->linktrait)->trait_id) << " ";
        outFile << (link->in_node)->node_id << " ";
        outFile << (link->out_node)->node_id << " ";
        outFile << (link->weight) << " ";
        outFile << (link->is_recurrent) << " ";
        outFile << innovation_num << " ";
        outFile << mutation_num << " ";
        outFile << enable << std::endl;
    }

    Gene::~Gene()
    {
    }

} // namespace Neat
