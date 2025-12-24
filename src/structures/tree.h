#pragma once
#include <string>
#include <vector>

using std::string; using std::vector;


class TreeNode {
public:
    string value;
    TreeNode* left;
    TreeNode* right;
    
    TreeNode(const string& val) : value(val), left(nullptr), right(nullptr) {}
};

class BinaryTree {
private:
    TreeNode* root;
    
    TreeNode* insertRecursive(TreeNode* node, const string& value);
    bool findRecursive(TreeNode* node, const string& value, string& path, int& position, int& currentPos) const;
    void clearRecursive(TreeNode* node);
    void inorderTraversal(TreeNode* node, int& position) const;
    TreeNode* copyRecursive(TreeNode* node);
    bool findPath(TreeNode* node, const string& value, string& path) const;

public:
    BinaryTree();
    ~BinaryTree();
    BinaryTree(const BinaryTree& other);
    BinaryTree& operator=(const BinaryTree& other);
    
    void insert(const string& value);
    bool find(const string& value, string& path, int& position) const;
    
    bool empty() const;
    void clear();
    
    void serializeToArray(TreeNode* node, vector<string>& result) const;
    TreeNode* deserializeFromArray(const vector<string>& data, int& index);
    
    friend struct Database;
};