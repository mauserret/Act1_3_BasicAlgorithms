/**
 * Program: AVL tree that stores the access counts of IP addresses, keyed by
 *          the numeric value of each IP. It supports insertion, range queries
 *          and top-N most accessed queries.
 * Authors: A01648241
 *          A01642638
 *          A01648221
 *          A01643073
 * Date: 31/10/2026
 *
 * Complexity notation used in the comments of this file:
 *   n = number of distinct IPs (nodes) stored in the tree,
 *   m = number of times insert is called (log lines read),
 *   k = number of nodes that fall inside a queried range,
 *   N = number of top nodes requested.
 * An AVL tree keeps its height under 1.44 * log2(n + 2), so every walk from
 * the root to a leaf costs O(log n).
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

/**
 * Node of the AVL tree. Holds the accumulated statistics of one distinct IP.
 */
struct AVLNode {
	long long ip;
	std::string strIp;
	int totalAccesses;
	int totalSuccess;
	int height;
	AVLNode* left;
	AVLNode* right;
};

class AVL {
public:
	/**
	 * @brief Creates an empty tree.
	 * @note Time O(1). Space O(1).
	 */
	AVL() = default;

	/**
	 * @brief Releases every node of the tree.
	 * @note Time O(n). Space O(log n) for the recursion stack.
	 */
	~AVL() {
		destroy(root);
	}

	// The tree owns raw pointers, so copying it is forbidden.
	AVL(const AVL&) = delete;
	AVL& operator=(const AVL&) = delete;

	/**
	 * @brief Gets the height of the whole tree.
	 * @return Height of the root, or -1 when the tree is empty.
	 * @note Time O(1) because every node stores its own height. Space O(1).
	 */
	int height() {
		return height(root);
	}

	/**
	 * @brief Registers one access of an IP. Creates the node if the IP is new,
	 *        otherwise increments its counters.
	 * @param searchIp Numeric value of the IP, used as the tree key.
	 * @param strIp Text form of the IP, stored when the node is created.
	 * @param successCount 1 if the access was successful, 0 otherwise.
	 * @note Time O(log n) in the worst case: one root-to-leaf walk with O(1)
	 *       work per level. Inserting m accesses costs O(m log n).
	 *       Space O(log n) for the recursion stack.
	 */
	void insert(long long searchIp, const std::string& strIp, int successCount) {
		root = insert(root, searchIp, strIp, successCount);
		++totalRecorded;
	}

	/**
	 * @brief Gets how many different IPs are stored.
	 * @return Number of nodes in the tree.
	 * @note Time O(1). Space O(1).
	 */
	int getDistinctIps() const {
		return distinctIps;
	}

	/**
	 * @brief Gets how many accesses have been inserted in total.
	 * @return Number of calls made to insert.
	 * @note Time O(1). Space O(1).
	 */
	int getTotalRecorded() const {
		return totalRecorded;
	}

	/**
	 * @brief Prints every IP inside a range, followed by a summary.
	 * @param ipRangeStart Numeric value of the first IP of the range.
	 * @param ipRangeEnd Numeric value of the last IP of the range.
	 * @param out Output stream where the IPs and the summary are printed.
	 * @note Time O(log n + k): only the two boundary paths and the k nodes
	 *       inside the range are visited. Space O(log n) for the recursion stack.
	 */
	void rangeQuery(long long ipRangeStart, long long ipRangeEnd, std::ostream& out) {
		int distinct = 0;
		long long totalAccesses = 0;

		rangeQuery(root, ipRangeStart, ipRangeEnd, distinct, totalAccesses, out);
		out << "Distinct IPs: " << distinct << "\n"
			<< "Total accesses: " << totalAccesses << "\n";
	}

	/**
	 * @brief Finds the most accessed IPs.
	 * @param n How many IPs are requested.
	 * @return Nodes sorted from the most to the least accessed, at most n of
	 *         them. Empty when n is not positive.
	 * @note Time O(n * N) in the worst case, see collectTop. Linear in the number
	 *       of distinct IPs when N is a constant. Space O(N) for the result
	 *       plus O(log n) for the recursion stack.
	 */
	std::vector<AVLNode*> nMostAccessed(int n) {
		return nMostAccessed(root, n);
	}

private:
	static constexpr int NULL_HEIGHT = -1;
	static constexpr int LEFT_HEAVY = -2;
	static constexpr int RIGHT_HEAVY = 2;

	AVLNode* root = nullptr;
	int distinctIps = 0;
	int totalRecorded = 0;

	/**
	 * @brief Gets the height of a subtree.
	 * @param node Root of the subtree, may be null.
	 * @return Height of the subtree, or -1 when it is empty.
	 * @note Time O(1). Space O(1).
	 */
	int height(AVLNode* node) {
		return node == nullptr ? NULL_HEIGHT : node->height;
	}

	/**
	 * @brief Computes the balance factor of a node.
	 * @param node Non-null node to evaluate.
	 * @return Height of the right subtree minus height of the left subtree.
	 * @note Time O(1). Space O(1).
	 */
	int balanceFactor(AVLNode* node) {
		return height(node->right) - height(node->left);
	}

	/**
	 * @brief Recalculates the stored height of a node from its children.
	 * @param node Non-null node whose children already have a correct height.
	 * @note Time O(1). Space O(1).
	 */
	void update(AVLNode* node) {
		node->height = 1 + std::max(height(node->left), height(node->right));
	}

	/**
	 * @brief Rotates a right-heavy subtree to the left.
	 * @param a Root of the right-heavy subtree.
	 * @return New root of the subtree after the rotation.
	 * @note Time O(1): only a constant number of pointers change. Space O(1).
	 */
	AVLNode* rotateLeft(AVLNode* a) {
		AVLNode* b = a->right;

		a->right = b->left;
		b->left = a;
		// a goes first because it is now below b
		update(a);
		update(b);
		return b;
	}

	/**
	 * @brief Rotates a left-heavy subtree to the right.
	 * @param a Root of the left-heavy subtree.
	 * @return New root of the subtree after the rotation.
	 * @note Time O(1): only a constant number of pointers change. Space O(1).
	 */
	AVLNode* rotateRight(AVLNode* a) {
		AVLNode* b = a->left;

		a->left = b->right;
		b->right = a;
		update(a);
		update(b);
		return b;
	}

	/**
	 * @brief Updates the height of a node and restores the AVL property with a
	 *        single or double rotation when its subtrees differ by 2.
	 * @param node Root of the subtree to rebalance.
	 * @return New root of the subtree.
	 * @note Time O(1): at most two rotations. Space O(1).
	 */
	AVLNode* rebalance(AVLNode* node) {
		update(node);
		if (balanceFactor(node) == RIGHT_HEAVY) {
			if (balanceFactor(node->right) < 0) {
				node->right = rotateRight(node->right);
			}
			return rotateLeft(node);
		}
		if (balanceFactor(node) == LEFT_HEAVY) {
			if (balanceFactor(node->left) > 0) {
				node->left = rotateLeft(node->left);
			}
			return rotateRight(node);
		}
		return node;
	}

	/**
	 * @brief Inserts an access into a subtree and rebalances on the way back.
	 * @param node Root of the subtree where the IP is inserted.
	 * @param searchIp Numeric value of the IP, used as the key.
	 * @param strIp Text form of the IP, stored when a node is created.
	 * @param successCount 1 if the access was successful, 0 otherwise.
	 * @return New root of the subtree.
	 * @note Time O(log n): the recursion follows one root-to-leaf path and each
	 *       level does O(1) work (compare, rebalance). An IP that already exists
	 *       stops at its node but still rebalances upward in O(1) per level.
	 *       Space O(log n) for the recursion stack.
	 */
	AVLNode* insert(AVLNode* node, long long searchIp, const std::string& strIp, int successCount) {
		if (node == nullptr) {
			++distinctIps;
			return new AVLNode{searchIp, strIp, 1, successCount, 0, nullptr, nullptr};
		}
		if (searchIp < node->ip) {
			node->left = insert(node->left, searchIp, strIp, successCount);
		} else if (searchIp > node->ip) {
			node->right = insert(node->right, searchIp, strIp, successCount);
		} else {
			node->totalAccesses += 1;
			node->totalSuccess += successCount;
		}
		return rebalance(node);
	}

	/**
	 * @brief In-order walk that prints only the nodes inside a range.
	 * @param node Root of the subtree being visited.
	 * @param ipRangeStart Numeric value of the first IP of the range.
	 * @param ipRangeEnd Numeric value of the last IP of the range.
	 * @param distinctInRange Counter of IPs found in the range, updated here.
	 * @param accessesInRange Sum of accesses of the IPs found, updated here.
	 * @param out Output stream where each IP found is printed.
	 * @note Time O(log n + k): the left subtree is skipped when the node is not
	 *       above the start and the right subtree when it is not below the end,
	 *       so only the two boundary paths (O(log n)) and the k matches are
	 *       visited. Space O(log n) for the recursion stack.
	 */
	void rangeQuery(AVLNode* node, long long ipRangeStart, long long ipRangeEnd,
					int& distinctInRange, long long& accessesInRange, std::ostream& out) {
		if (node == nullptr) {
			return;
		}
		if (node->ip > ipRangeStart) {
			rangeQuery(node->left, ipRangeStart, ipRangeEnd, distinctInRange, accessesInRange, out);
		}
		if (ipRangeStart <= node->ip && node->ip <= ipRangeEnd) {
			out << node->strIp << " | Accesses: " << node->totalAccesses << "\n";
			++distinctInRange;
			accessesInRange += node->totalAccesses;
		}
		if (node->ip < ipRangeEnd) {
			rangeQuery(node->right, ipRangeStart, ipRangeEnd, distinctInRange, accessesInRange, out);
		}
	}

	/**
	 * @brief Walks the tree in order and keeps a sorted board with the most
	 *        accessed nodes found so far.
	 * @param node Root of the subtree being visited.
	 * @param limit Maximum number of nodes kept on the board.
	 * @param board Nodes sorted from most to least accessed, updated here.
	 * @note Time O(n * N) in the worst case: all n nodes are visited and each one
	 *       may shift up to N board entries. It is O(n) when N is a constant
	 *       (5 or 15 in the monitor) but approaches O(n^2) when N is close to n;
	 *       a min-heap would give O(n log N). Space O(N) for the board plus
	 *       O(log n) for the recursion stack.
	 */
	void collectTop(AVLNode* node, std::size_t limit, std::vector<AVLNode*>& board) {
		if (node == nullptr) {
			return;
		}
		collectTop(node->left, limit, board);

		bool full = board.size() == limit;

		// Only a node stronger than the weakest one on a full board gets in
		if (!full || node->totalAccesses > board.back()->totalAccesses) {
			board.push_back(node);
			std::size_t position = board.size() - 1;

			while (position > 0 && board[position - 1]->totalAccesses < node->totalAccesses) {
				board[position] = board[position - 1];
				--position;
			}
			board[position] = node;
			if (board.size() > limit) {
				board.pop_back();
			}
		}
		collectTop(node->right, limit, board);
	}

	/**
	 * @brief Builds the board of most accessed nodes of a subtree.
	 * @param node Root of the subtree to inspect.
	 * @param n How many nodes are requested.
	 * @return Nodes sorted from most to least accessed, at most n of them.
	 *         Empty when n is not positive.
	 * @note Time O(n * N) as in collectTop. Space O(N) plus O(log n) for the stack.
	 */
	std::vector<AVLNode*> nMostAccessed(AVLNode* node, int n) {
		std::vector<AVLNode*> board = {};

		if (n > 0) {
			collectTop(node, n, board);
		}
		return board;
	}

	/**
	 * @brief Deletes a subtree in post-order.
	 * @param node Root of the subtree to delete, may be null.
	 * @note Time O(n): every node is deleted once. Space O(log n) for the
	 *       recursion stack.
	 */
	void destroy(AVLNode* node) {
		if (node == nullptr) {
			return;
		}
		destroy(node->left);
		destroy(node->right);
		delete node;
	}
};