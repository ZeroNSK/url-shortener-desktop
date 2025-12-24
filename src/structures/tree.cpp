#include "tree.h"
#include <iostream>
#include <string>
#include <vector>


using std::vector;using std::string;

BinaryTree::BinaryTree() {
    root = nullptr;
}
BinaryTree::~BinaryTree() {
    clear();
}

TreeNode* BinaryTree::copyRecursive(TreeNode* node) {
    
    if (node == nullptr) 
        return nullptr;

    TreeNode* newNode = new TreeNode(node->value);

    newNode->left = copyRecursive(node->left);
    newNode->right = copyRecursive(node->right);
    
    return newNode;
}

BinaryTree::BinaryTree(const BinaryTree& other) {
    root = copyRecursive(other.root); // копируем дерево
}

BinaryTree& BinaryTree::operator=(const BinaryTree& other) {
    if (this != &other) {
        clear(); 
        root = copyRecursive(other.root);
    }
    
    return *this;
}

bool BinaryTree::empty() const {
    return root == nullptr; // тру если дерево пусто
}

TreeNode* BinaryTree::insertRecursive(TreeNode* node, const string& value) {
    
    if (node == nullptr) {
        return new TreeNode(value);
    }
    
    // если значение меньше идём влево, нет - вправо
    if (value < node->value) {
        node->left = insertRecursive(node->left, value);
    } else if (value > node->value) {
        node->right = insertRecursive(node->right, value);
    }
    
    return node;
}

void BinaryTree::insert(const string& value) {
    root = insertRecursive(root, value);
}

// Функция для подсчета позиции в inorder обходе
void BinaryTree::inorderTraversal(TreeNode* node, int& position) const {
    if (node == nullptr) return;
    
    inorderTraversal(node->left, position);
    position++;
    inorderTraversal(node->right, position);
}


//поиск позиции
bool BinaryTree::findRecursive(TreeNode* node, const string& value, string& path, int& position, int& currentPos) const {
    
    if (node == nullptr) 
        return false;
    
    // сначала влево
    if (findRecursive(node->left, value, path, position, currentPos)) {
        return true;
    }
    
    // работа с текущим узлом
    currentPos++;
    if (node->value == value) {
        position = currentPos;
        return true;
    }
    
    // потом вправо 
    if (findRecursive(node->right, value, path, position, currentPos)) {
        return true;
    }
    
    return false; 
}

//построение пути
bool BinaryTree::findPath(TreeNode* node, const string& value, string& path) const {
    if (node == nullptr) 
        return false;
    
    if (node->value == value) {
        return true;
    }

    if (value < node->value && node->left != nullptr) {
        path += "L";
        if (findPath(node->left, value, path)) {
            return true;
        }
        path.pop_back(); // убираем последнюю букву если не найден
    }
    
    if (value > node->value && node->right != nullptr) {
        path += "R";
        if (findPath(node->right, value, path)) {
            return true;
        }
        path.pop_back();
    }
    
    return false;
}

bool BinaryTree::find(const string& value, string& path, int& position) const {
    
    int currentPos = 0;
    position = 0; 
    
    // ищем поз в inorder обходе
    bool found = findRecursive(root, value, path, position, currentPos);
    
    if (found && root != nullptr) {
        path = "";
        if (root->value == value) {
            path = "корень";
        } else {
            // ищем путь от корня к элементу
            findPath(root, value, path);
            if (path.empty()) {
                path = "не найден";
            }
        }
    }
    
    return found;
}

void BinaryTree::clearRecursive(TreeNode* node) {
    
    if (node == nullptr)
        return;
    
    // удаляяем оба поддерева
    clearRecursive(node->left);
    clearRecursive(node->right);
    
    delete node; 
}

void BinaryTree::clear() {
    
    clearRecursive(root);
    root = nullptr;
}

// корень левое право
void BinaryTree::serializeToArray(TreeNode* node, vector<string>& result) const {
    if (node == nullptr) {
        result.push_back("null");
        return;
    }
    
    result.push_back(node->value);
    serializeToArray(node->left, result);
    serializeToArray(node->right, result);
}


// обратно восстанавливаем из массива
TreeNode* BinaryTree::deserializeFromArray(const vector<string>& data, int& index) {
    if (index >= data.size() || data[index] == "null") { 
        index++;
        return nullptr;
    }
    
    TreeNode* node = new TreeNode(data[index]); 
    index++;
    
    node->left = deserializeFromArray(data, index); 
    node->right = deserializeFromArray(data, index);
    
    return node;
}

void TINSERT(BinaryTree* t, const string& value) {
    t->insert(value);
}

bool TFIND(BinaryTree* t, const string& value, string& path, int& position) {
    return t->find(value, path, position);
}

void TREECLEAR(BinaryTree* t) {
    t->clear();
}