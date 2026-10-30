/**
 * Program: Benchmark that inserts ascending, descending and shuffled keys in
 *          an AVL tree and compares the resulting height and insertion time
 *          against log2(n). The height of an AVL tree never exceeds roughly
 *          1.44 * log2(n), even with sorted input.
 * Authors: A01648241
 *          A01642638
 *          A01648221
 *          A01643073
 * Date: 31/10/2026
 * Compilation string: g++ AVL_test.cpp -o AVL_test.exe
 * Run string: ./AVL_test.exe
*/

#include "AVL.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

const std::vector<long long> TEST_SIZES = {1000LL, 100000LL, 1000000LL};
const std::string DUMMY_IP_TEXT = "x";
const int DUMMY_SUCCESS = 1;

/**
 * @brief Inserts every key into a fresh tree, timing only the inserts, and
 *        prints the height reached.
 * @param label Name of the key order being tested.
 * @param keys Keys to insert, in the order they are inserted.
 * @note Time O(n log n) for n keys: n inserts of O(log n) each. Space O(n)
 *       for the tree.
 */
void runTest(const std::string& label, const std::vector<long long>& keys) {
	AVL tree{};
	auto start = std::chrono::steady_clock::now();

	for (long long key : keys) {
		tree.insert(key, DUMMY_IP_TEXT, DUMMY_SUCCESS);
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	std::cout << label << " | n = " << keys.size()
		<< " | height = " << tree.height()
		<< " | log2(n) = " << std::log2(static_cast<double>(keys.size()))
		<< " | time = " << elapsed.count() << " ms\n";
}

/**
 * @brief Entry point: runs the three key orders for every size in TEST_SIZES.
 * @return 0 when all the tests finish.
 */
int main() {
	std::mt19937 gen(std::random_device{}());

	for (long long n : TEST_SIZES) {
		std::vector<long long> ascending = {};

		for (long long i = 1; i <= n; ++i) {
			ascending.push_back(i);
		}

		std::vector<long long> descending(ascending.rbegin(), ascending.rend());
		std::vector<long long> shuffled = ascending;

		std::shuffle(shuffled.begin(), shuffled.end(), gen);

		runTest("Ascending ", ascending);
		runTest("Descending", descending);
		runTest("Shuffled  ", shuffled);
        std::cout << "\n";
	}
    std::cout << "No height should be greater than 1.44*log2(n)\n";
	return 0;
}