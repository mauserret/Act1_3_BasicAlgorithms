#pragma once
#include "Log.h"
#include <algorithm>

struct AVLNode
{
    long long ip;
    std::string strIp;
    int totalAccesses;
    int totalSuccess;
    int height;
    AVLNode* left;
    AVLNode* right;
};

class AVL
{
public:
    AVL() = default;
    ~AVL() { destroy(root); }


    AVL(const AVL &) = delete;
    AVL &operator=(const AVL &) = delete;

    // Each public method calls the private method with the same name, starting at root.
    void insert(long long seachIp, int totSuccess, std::string strIp) { root = insert(root, seachIp, totSuccess, strIp); }
    int height() { return height(root); }
private:
    AVLNode* root = nullptr;

    int height(AVLNode* n) { return n == nullptr ? -1 : n->height; }
    int bf(AVLNode* n) { return height(n->right) - height(n->left); }
    void update(AVLNode* n) {
        n->height = 1 + std::max(height(n->left), height(n->right));
    }
    AVLNode* rotateLeft(AVLNode* a) {     // a is right-heavy
        AVLNode* b = a->right;            // b rises
        a->right = b->left;               // T2 changes parent
        b->left = a;                      // a goes down to the left
        update(a);                        // a first: it is now below b
        update(b);
        return b;                         // new root of this subtree
    }
    AVLNode* rotateRight(AVLNode* a) {     
        AVLNode* b = a->left;            
        a->left = b->right;               
        b->right = a;                      
        update(a);                        
        update(b);
        return b;                         
    }
 
    AVLNode* rebalance(AVLNode* n) {
    update(n);
    if (bf(n) == 2) {
        if (bf(n->right) < 0)
            n->right = rotateRight(n->right);
        return rotateLeft(n);
    }
    else if (bf(n) == -2) {
        if (bf(n->left) < 0) n->left = rotateLeft(n->right);
        return rotateRight(n);
    }
    return n;
}
    AVLNode* insert(AVLNode* n, long long searchIp, int totSuccess, std::string strIp) {
        if (n == nullptr) {
            return new AVLNode{searchIp, strIp, 1, totSuccess, 0, nullptr, nullptr};
        }
        if (searchIp < n->ip) n->left = insert(n->left, searchIp, totSuccess, strIp);
        if (searchIp > n->ip) n->right = insert(n->right, searchIp, totSuccess, strIp);
        n->totalAccesses += 1;
        n->totalSuccess += totSuccess;
        return rebalance(n);
    }

    // O(log n + k), where k is the number of IPs in the range
    void rangeQuery(AVLNode* node, long long ipRangeStart, long long ipRangeEnd) {
        if (node == nullptr) return;
        if (node->ip > ipRangeStart) rangeQuery(node->left, ipRangeStart, ipRangeEnd);

        if ((ipRangeStart <= node->ip) && (node->ip <= ipRangeEnd)) {
            std::cout << node->strIp << " Total accesses: " << node->totalAccesses 
            << " Successful accesses: " << node->totalSuccess << "\n";
        }

        if (node->ip < ipRangeEnd) rangeQuery(node->right, ipRangeStart, ipRangeEnd);
    }

    void nMostAccessed(AVLNode* node, int n) {
        // TODO
    }

	void destroy(AVLNode *n) {
		if (n == nullptr) return;
		destroy(n->left);
		destroy(n->right);
		delete n;
	}
};

