#pragma once

#include <vector>
#include <cassert>
#include <string>

namespace yakka {

struct XGTreeNode
{
    std::string name = "";
    double condOrLeafVal = 0.0;
    int yesJump = -1; // -ve means leaf
    int noJump = -1;
    bool unset = true;
};

struct XGBooster
{
    void AddTreeNode(int id, std::string name, double cond, 
            int yes, int no, int /*miss*/)
    {
        tree.resize(1+std::max(id, (int)tree.size()));
        auto& node = tree[id];
        assert(node.unset);
        node.name = name;
        node.condOrLeafVal = cond;
        node.yesJump = yes;
        node.noJump = no;
        node.unset = false;
    }
    void AddTreeNode(int id, std::string name, int cond, 
            int yes, int no, int miss)
    {
        AddTreeNode(id, name, static_cast<double>(cond), yes, no, miss);
    }
    void AddLeaf(int id, double cond)
    {
        AddTreeNode(id, "leaf", cond, -1, -1, -1);
    }
    void AddLeaf(int id, int cond)
    {
        AddTreeNode(id, "leaf", cond, -1, -1, -1);
    }
    std::vector<XGTreeNode> tree;
};

using XGBoosters = std::vector<XGBooster>;

};
