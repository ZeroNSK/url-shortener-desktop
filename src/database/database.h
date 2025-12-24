#pragma once
#include <string>
#include <map>
#include "../structures/set.h"
#include "../structures/stack.h"
#include "../structures/queue.h"
#include "../structures/hash.h"
#include "../structures/tree.h"

using std::string;
using std::map;

struct Database {
    map<string, Set>         sets;    
    map<string, Stack>       stacks;   
    map<string, Queue>       queues;
    map<string, Hash>        hashes;
    map<string, BinaryTree>  trees;

    bool load(const string& path);

    bool save(const string& path);
};