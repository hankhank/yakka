#pragma once

struct XGTreeNode
{
    std::string id;
    double condOrLeafVal = 0.0;
    int yesJump = -1; // -ve means leaf
    int noJump = -1;
};

struct XGBooster
{
    std::vector<XGTreeNode> tree;
}

typedef std::vector<XGBooster> XGBoosters;

