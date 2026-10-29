#pragma once
#include "Log.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include<cstddef>

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
    void insert(long long searchIp, int totSuccess, std::string strIp) { root = insert(root, searchIp, totSuccess, strIp); }
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
    if (bf(n) == -2) {
        if (bf(n->left) > 0) 
            n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    return n;
}
    AVLNode* insert(AVLNode* n, long long searchIp, int totSuccess, const std::string& strIp) {
        if (n == nullptr) {
            return new AVLNode{searchIp, strIp, 1, totSuccess, 0, nullptr, nullptr};
        }
        if (searchIp < n->ip) n->left = insert(n->left, searchIp, totSuccess, strIp);
        if (searchIp > n->ip) n->right = insert(n->right, searchIp, totSuccess, strIp);
        if (searchIp == n->ip) {
            n->totalAccesses += 1;
            n->totalSuccess += totSuccess;
        }   
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

    // O(n * N) where n is the number of nodes in the tree and N is the limit (or top nodes wished to view).
    // its linear in the number of distinct IP's because N is fixed.
    // Space complexity of O(N) for the board plus O(log(n)) for the recursion stack 
    void collectTop(AVLNode* node, std::size_t limit, std::vector<AVLNode*>& board) {
        if (node == nullptr) return;
        collectTop(node->left, limit, board);

        bool full = board.size() == limit;
        // if the board is not full or the current node is stronger than the weakest node in the array then make space for it
        if (!full || node->totalAccesses > board.back()->totalAccesses) {
            board.push_back(node); // temporary slot at the bottom
            std::size_t i = board.size() - 1;

            while (i > 0 && board[i - 1]->totalAccesses < node->totalAccesses) {
                board[i] = board[i - 1]; // Move the weaker node down until the current node is no longer stronger
                --i;
            }
            board[i] = node; // Set the node in its correct position to mantain the board's order
            if (board.size() > limit) board.pop_back(); // Pop the weakest node out if the board is full

        }
        collectTop(node->right, limit, board);
    }

    std::vector<AVLNode*> nMostAccessed(AVLNode* node, int n) {
        std::vector<AVLNode*> board;
        if (n > 0) collectTop(node, n, board);
        return board;
    }

	void destroy(AVLNode *n) {
		if (n == nullptr) return;
		destroy(n->left);
		destroy(n->right);
		delete n;
	}
};

