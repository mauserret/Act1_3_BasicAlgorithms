#include "AVL.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// Inserts every key into a fresh tree, timing only the inserts, and reports the height.
void runTest(const std::string& label, const std::vector<long long>& keys) {
    AVL tree;

    auto start = std::chrono::steady_clock::now();
    for (long long key : keys) {
        tree.insert(key, 1, "x");
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << label << " | n = " << keys.size()
              << " | height = " << tree.height()
              << " | log2(n) = " << std::log2(static_cast<double>(keys.size()))
              << " | time = " << elapsed.count() << " ms\n";
}

int main() {
    std::mt19937 gen(std::random_device{}());

    for (long long n : {1000LL, 100000LL}) {
        std::vector<long long> ascending;
        for (long long i = 1; i <= n; ++i) ascending.push_back(i);

        std::vector<long long> descending(ascending.rbegin(), ascending.rend());

        std::vector<long long> shuffled = ascending;
        std::shuffle(shuffled.begin(), shuffled.end(), gen);

        runTest("Ascending ", ascending);
        runTest("Descending", descending);
        runTest("Shuffled  ", shuffled);
        std::cout << "\n";
    }
    return 0;
}

// AVL’s height never exceeds roughly 1.44 · log₂(n)