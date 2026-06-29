#include <fstream>

#include "Gene.h"

namespace Neat
{
    Gene::Gene(/* args */)
    {
    }

    Gene::Gene(const Neat &neat,
               double w,
               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
               bool recur, double innov, double mnum)
    {
        link = std::make_shared<Link>(neat, w, inode, onode, recur);
        innovation_num = innov;
        mutation_num = mnum;

        enable = true;

        frozen = false;
    }

    Gene::Gene(const Neat &neat,
               std::shared_ptr<Trait> tp, double w,
               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode,
               bool recur, double innov, double mnum)
    {
        link = std::make_shared<Link>(neat, tp, w, inode, onode, recur);
        innovation_num = innov;
        mutation_num = mnum;

        enable = true;

        frozen = false;
    }

    Gene::Gene(const Neat &neat,
               const Gene &g,
               std::shared_ptr<Trait> tp,
               std::shared_ptr<NNode> inode, std::shared_ptr<NNode> onode)
    {
        // cout<<"Trying to attach nodes: "<<inode<<" "<<onode<<endl;
        link = std::make_shared<Link>(neat, tp, g.link->weight, inode, onode, g.link->is_recurrent);
        innovation_num = g.innovation_num;
        mutation_num = g.mutation_num;
        enable = g.enable;

        frozen = g.frozen;
    }

    Gene::Gene(const Gene &gene)
    {
        innovation_num = gene.innovation_num;
        mutation_num = gene.mutation_num;
        enable = gene.enable;
        frozen = gene.frozen;

        link = std::make_shared<Link>(gene.link.get());
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
