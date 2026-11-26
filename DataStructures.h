#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <string>
#include <stdexcept>
#include <cstddef>
#include "Building.h"
#include "Containers.h"
using namespace std;
const int MAX_BUILDINGS = 100;
struct BNode {
    Building* data;
    BNode* next;
};
class BuildingList {
    BNode* head;
    int size;
    BuildingList(const BuildingList&);
    BuildingList& operator=(const BuildingList&);
public:
    BuildingList() : head(NULL), size(0) {}
    ~BuildingList() {
        while (head != NULL) {
            BNode* temp = head;
            head = head->next;
            delete temp->data;
            delete temp;
        }
    }
    int getSize() const { return size; }
    BNode* getHead() const { return head; }

    void add(Building* b) {
        if (size >= MAX_BUILDINGS)
            throw runtime_error("Building limit reached.");
        BNode* node = new BNode;
        node->data = b;
        node->next = NULL;
        if (head == NULL)
            head = node;
        else {
            BNode* cur = head;
            while (cur->next != NULL)
                cur = cur->next;
            cur->next = node;
        }
        size++;
    }
    void remove(const string& name) {
        BNode* prev = NULL;
        BNode* cur = head;
        while (cur != NULL && cur->data->getName() != name) {
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL)
            throw runtime_error("Building '" + name + "' not found.");
        if (prev == NULL)
            head = cur->next;
        else
            prev->next = cur->next;
        delete cur->data;
        delete cur;
        size--;
    }
};
class BuildingTable {
    static const int SIZE = 11;
    struct HNode {
        string key;
        Building* value;
        HNode* next;
    };
    HNode* table[SIZE];
    int hashKey(const string& key) const {
        int h = 0;
        for (size_t i = 0; i < key.length(); i++)
            h = (h * 31 + key[i]) % SIZE;
        if (h < 0)
            h += SIZE;
        return h;
    }
    BuildingTable(const BuildingTable&);
    BuildingTable& operator=(const BuildingTable&);
public:
    BuildingTable() {
        for (int i = 0; i < SIZE; i++)
            table[i] = NULL;
    }
    ~BuildingTable() {
        for (int i = 0; i < SIZE; i++) {
            while (table[i] != NULL) {
                HNode* temp = table[i];
                table[i] = table[i]->next;
                delete temp;
            }
        }
    }
    void insert(Building* b) {
        string key = b->getName();
        int idx = hashKey(key);
        for (HNode* cur = table[idx]; cur != NULL; cur = cur->next)
            if (cur->key == key)
                throw invalid_argument("A building with this name already exists.");
        HNode* node = new HNode;
        node->key = key;
        node->value = b;
        node->next = table[idx];
        table[idx] = node;
    }
    bool contains(const string& key) const {
        int idx = hashKey(key);
        for (HNode* cur = table[idx]; cur != NULL; cur = cur->next)
            if (cur->key == key)
                return true;
        return false;
    }
    Building* find(const string& key) const {
        int idx = hashKey(key);
        for (HNode* cur = table[idx]; cur != NULL; cur = cur->next)
            if (cur->key == key)
                return cur->value;
        throw runtime_error("Building '" + key + "' not found (names are case-sensitive).");
    }
    void remove(const string& key) {
        int idx = hashKey(key);
        HNode* prev = NULL;
        HNode* cur = table[idx];
        while (cur != NULL && cur->key != key) {
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL)
            throw runtime_error("Building '" + key + "' not found (names are case-sensitive).");
        if (prev == NULL)
            table[idx] = cur->next;
        else
            prev->next = cur->next;
        delete cur;
    }
};
class BuildingTree {
    struct TNode {
        Building* data;
        TNode* left;
        TNode* right;
    };
    TNode* root;
    TNode* insertNode(TNode* node, Building* b) {
        if (node == NULL) {
            TNode* fresh = new TNode;
            fresh->data = b;
            fresh->left = NULL;
            fresh->right = NULL;
            return fresh;
        }
        if (b->getName() < node->data->getName())
            node->left = insertNode(node->left, b);
        else
            node->right = insertNode(node->right, b);
        return node;
    }
    void inOrder(TNode* node, Building* out[], int& n, int maxCount) const {
        if (node == NULL)
            return;
        inOrder(node->left, out, n, maxCount);
        if (n < maxCount)
            out[n++] = node->data;
        inOrder(node->right, out, n, maxCount);
    }
    void destroy(TNode* node) {
        if (node == NULL)
            return;
        destroy(node->left);
        destroy(node->right);
        delete node;                    
    }
    BuildingTree(const BuildingTree&);
    BuildingTree& operator=(const BuildingTree&);

public:
    BuildingTree() : root(NULL) {}
    ~BuildingTree() { destroy(root); }

    void insert(Building* b) { root = insertNode(root, b); }

    int sortedList(Building* out[], int maxCount) const {
        int n = 0;
        inOrder(root, out, n, maxCount);
        return n;
    }
};
struct GridNode;

struct GridEdge {
    GridNode* to;
    GridEdge* next;
};
struct GridNode {
    string name;
    GridEdge* edges;      
    GridNode* next;       
    bool visited;        
};
class PowerGrid {
    GridNode* nodes;
    int nodeTotal;
    int linkTotal;

    GridNode* findNode(const string& name) const {
        for (GridNode* cur = nodes; cur != NULL; cur = cur->next)
            if (cur->name == name)
                return cur;
        return NULL;
    }
    GridNode* findOrCreate(const string& name) {
        GridNode* node = findNode(name);
        if (node != NULL)
            return node;
        node = new GridNode;
        node->name = name;
        node->edges = NULL;
        node->next = NULL;
        node->visited = false;
        if (nodes == NULL)
            nodes = node;
        else {
            GridNode* cur = nodes;
            while (cur->next != NULL)
                cur = cur->next;
            cur->next = node;
        }
        nodeTotal++;
        return node;
    }
    PowerGrid(const PowerGrid&);
    PowerGrid& operator=(const PowerGrid&);
public:
    PowerGrid() : nodes(NULL), nodeTotal(0), linkTotal(0) {}
    ~PowerGrid() { clear(); }
    void clear() {
        while (nodes != NULL) {
            GridNode* temp = nodes;
            nodes = nodes->next;
            while (temp->edges != NULL) {
                GridEdge* e = temp->edges;
                temp->edges = e->next;
                delete e;
            }
            delete temp;
        }
        nodeTotal = 0;
        linkTotal = 0;
    }
    int linkList(string from[], string to[], int maxCount) const {
        int n = 0;
        for (GridNode* cur = nodes; cur != NULL; cur = cur->next)
            for (GridEdge* e = cur->edges; e != NULL && n < maxCount; e = e->next) {
                from[n] = cur->name;
                to[n] = e->to->name;
                n++;
            }
        return n;
    }
    int getNodeCount() const { return nodeTotal; }
    int getLinkCount() const { return linkTotal; }
    bool hasNode(const string& name) const { return findNode(name) != NULL; }
    void addLink(const string& from, const string& to) {
        if (from.empty() || to.empty())
            throw invalid_argument("Both names are needed.");
        if (from == to)
            throw invalid_argument("A point cannot feed itself.");
        if (from.find('|') != string::npos || to.find('|') != string::npos)
            throw invalid_argument("The character | is not allowed in names.");
        GridNode* a = findOrCreate(from);
        GridNode* b = findOrCreate(to);
        GridEdge* last = NULL;
        for (GridEdge* e = a->edges; e != NULL; e = e->next) {
            if (e->to == b)
                throw invalid_argument("That link already exists.");
            last = e;
        }
        GridEdge* edge = new GridEdge;
        edge->to = b;
        edge->next = NULL;
        if (last == NULL)
            a->edges = edge;
        else
            last->next = edge;
        linkTotal++;
    }
    int affectedBy(const string& source, string out[], int maxCount) {
        GridNode* start = findNode(source);
        if (start == NULL)
            throw runtime_error("'" + source + "' is not in the power network.");
        for (GridNode* n = nodes; n != NULL; n = n->next)
            n->visited = false;

        Queue<GridNode*> waiting;
        start->visited = true;
        waiting.enqueue(start);
        int found = 0;
        while (!waiting.isEmpty()) {
            GridNode* current = waiting.dequeue();
            for (GridEdge* e = current->edges; e != NULL; e = e->next) {
                if (!e->to->visited) {
                    e->to->visited = true;
                    if (found < maxCount)
                        out[found++] = e->to->name;
                    waiting.enqueue(e->to);
                }
            }
        }
        return found;
    }
    int describe(string out[], int maxCount) const {
        int n = 0;
        for (GridNode* cur = nodes; cur != NULL && n < maxCount; cur = cur->next) {
            string line = cur->name;
            if (cur->edges == NULL) {
                line += "  (end point)";
            } else {
                line += " -> ";
                for (GridEdge* e = cur->edges; e != NULL; e = e->next) {
                    line += e->to->name;
                    if (e->next != NULL)
                        line += ", ";
                }
            }
            out[n++] = line;
        }
        return n;
    }
};

#endif
