/**
 * Program: Access log monitor. Reads a date-sorted log file, stores the
 *          accesses of every IP in an AVL tree, prints a report at the end of
 *          each month and then lets the user query IP ranges and the top N
 *          most accessed IPs. Reports are written to the console and to
 *          output.txt.
 * Authors: A01648241
 *          A01642638
 *          A01648221
 *          A01643073
 * Date: 31/10/2026
 *
 * Complexity notation: n = distinct IPs in the tree, m = log lines read,
 * k = IPs inside a queried range, N = number of top IPs requested,
 * M = number of months in the log.
 * 
 * Compilation string: g++ logsMonitor.cpp -o monitorMain.exe
 * Run string: ./monitorMain.exe orderedLogsByDate.txt  
 */

#include "AVL.h"
#include "IpUtils.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

const int MONTHLY_TOP_COUNT = 5;
const int FINAL_TOP_COUNT = 15;
const int IGNORE_LIMIT = 10000;
const int INVALID_OPTION = -1;
const int RANGE_OPTION = 1;
const int TOP_OPTION = 2;
const int EXIT_OPTION = 3;
const std::string OUTPUT_FILE_NAME = "output.txt";
const std::string SUCCESS_PREFIX = "Accepted";

/**
 * @brief Prints the N most accessed IPs with their success percentage.
 * @param n How many IPs are printed.
 * @param tree Tree that holds the IP statistics.
 * @param out Output stream where the list is printed.
 * @note Time O(n * N) in the worst case, O(n) for a constant N, because of
 *       nMostAccessed. Space O(N).
 */
void printTopNodes(int n, AVL& tree, std::ostream& out) {
	out << "\n --- Top " << n << " IPs ---\n";
	for (AVLNode* node : tree.nMostAccessed(n)) {
		double percentage = 100.0 * node->totalSuccess / node->totalAccesses;

		out << node->strIp
			<< " | Accesses: " << node->totalAccesses
			<< " | Success: " << node->totalSuccess
			<< " | Success %: " << std::fixed << std::setprecision(2) << percentage << "\n";
	}
}

/**
 * @brief Prints the report of a month: distinct IPs, average accesses per IP
 *        and the five most accessed IPs.
 * @param month Name of the month being reported.
 * @param tree Tree that holds the IP statistics up to the end of that month.
 * @param out Output stream where the report is printed.
 * @note Time O(n) because the top five query is linear for a constant N.
 *       Space O(1).
 */
void printMonthlyReport(const std::string& month, AVL& tree, std::ostream& out) {
	int distinct = tree.getDistinctIps();
	double average = 0.0;

	if (distinct > 0) {
		average = static_cast<double>(tree.getTotalRecorded()) / distinct;
	}
	out << "\n --- End of month " << month << " ---\n"
		<< "Distinct IPs: " << distinct << "\n"
		<< "Average accesses per IP: " << std::fixed << std::setprecision(2) << average << "\n"
		<< "Top " << MONTHLY_TOP_COUNT << " IPs:\n";
	for (AVLNode* node : tree.nMostAccessed(MONTHLY_TOP_COUNT)) {
		out << "  " << node->strIp << " | Accesses: " << node->totalAccesses << "\n";
	}
}

/**
 * @brief Prints the header and the result of a range query.
 * @param tree Tree that holds the IP statistics.
 * @param startIp First IP of the range, as text.
 * @param endIp Last IP of the range, as text.
 * @param out Output stream where the result is printed.
 * @note Time O(log n + k). Space O(log n).
 */
void printRange(AVL& tree, const std::string& startIp, const std::string& endIp, std::ostream& out) {
	out << "\n--- IPs in range: " << startIp << " - " << endIp << " ---\n";
	tree.rangeQuery(getTotalIp(startIp), getTotalIp(endIp), out);
}

/**
 * @brief Finds the most accessed IP and queries its whole x.y.z.0 - x.y.z.255 block.
 * @param tree Tree that holds the IP statistics.
 * @param out Output stream where the result is printed.
 * @note Time O(n + log n + k): one top-1 query plus one range query.
 *       Space O(log n).
 */
void printTopIpBlock(AVL& tree, std::ostream& out) {
	std::vector<AVLNode*> top = tree.nMostAccessed(1);

	if (!top.empty()) {
		AVLNode* topNode = top[0];
		std::string prefix = topNode->strIp.substr(0, topNode->strIp.rfind('.'));

		out << "\nTop IP: " << topNode->strIp << " -> querying its block";
		printRange(tree, prefix + ".0", prefix + ".255", out);
	}
}

/**
 * @brief Tells whether a log message reports a successful access.
 * @param msg Message part of a log line.
 * @return 1 if the message starts with "Accepted", 0 otherwise.
 * @note Time O(1) because only the prefix is compared. Space O(1).
 */
int getSuccessFlag(const std::string& msg) {
	return msg.rfind(SUCCESS_PREFIX, 0) == 0 ? 1 : 0;
}

/**
 * @brief Reads the log file, inserts every access in a new AVL tree and prints
 *        the report of each month when the month changes.
 * @param fileName Path of the log file.
 * @param outFile Output file where the monthly reports are also written.
 * @return Tree with all the accesses, or nullptr if the file cannot be opened.
 *         The caller must delete it.
 * @note Time O(m log n + M * n): m inserts of O(log n) each, plus one O(n)
 *       report per month. Space O(n) for the tree.
 */
AVL* getAVLTree(const std::string& fileName, std::ofstream& outFile) {
	std::ifstream file(fileName);

	if (!file.is_open()) {
		std::cout << "Couldn't open the file \n";
		return nullptr;
	}

	AVL* tree = new AVL();
	std::string previousMonth = "";
	std::string line = "";

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string month = "";
		std::string ignoredField = "";
		std::string ip = "";
		std::string msg = "";

		// Day, time and host are not needed by the monitor
		std::getline(ss, month, ' ');
		std::getline(ss, ignoredField, ' ');
		std::getline(ss, ignoredField, ':');
		std::getline(ss, ignoredField, ':');
		std::getline(ss, ignoredField, ' ');
		std::getline(ss, ip, ' ');
		std::getline(ss, ignoredField, ' ');
		std::getline(ss, msg);

		if (!previousMonth.empty() && month != previousMonth) {
			printMonthlyReport(previousMonth, *tree, std::cout);
			printMonthlyReport(previousMonth, *tree, outFile);
		}
		previousMonth = month;

		long long numericIp = getTotalIp(ip);
		std::string address = ip.substr(0, ip.find(':'));

		tree->insert(numericIp, address, getSuccessFlag(msg));
	}

	// The last month never sees a "next month" line, so it is reported here
	if (!previousMonth.empty()) {
		printMonthlyReport(previousMonth, *tree, std::cout);
		printMonthlyReport(previousMonth, *tree, outFile);
	}
	return tree;
}

/**
 * @brief Checks that a numeric range is not reversed.
 * @param startRange First value of the range.
 * @param endRange Last value of the range.
 * @return true if endRange is greater than or equal to startRange.
 * @note Time O(1). Space O(1).
 */
template <typename T>
bool validSearchRange(const T& startRange, const T& endRange) {
	return endRange >= startRange;
}

/**
 * @brief Prints the menu of the monitor.
 * @note Time O(1). Space O(1).
 */
void printMenu() {
	std::cout << "\nSelect an option: " << "\n"
		<< RANGE_OPTION << ". Search by IP range" << "\n"
		<< TOP_OPTION << ". Get top n accessed IPs" << "\n"
		<< EXIT_OPTION << ". Exit\n"
		<< "Option: ";
}

/**
 * @brief Reads the option typed by the user.
 * @return The option read, or INVALID_OPTION if the input is not a number.
 * @note Time O(1). Space O(1).
 */
int readOption() {
	int option = INVALID_OPTION;

	if (!(std::cin >> option)) {
		std::cin.clear();
		std::cin.ignore(IGNORE_LIMIT, '\n');
		option = INVALID_OPTION;
	}
	return option;
}

/**
 * @brief Asks for a start and an end IP and prints the IPs between them.
 * @param tree Tree that holds the IP statistics.
 * @note Time O(log n + k). Space O(log n). Invalid IPs or a reversed range
 *       are reported as an error message.
 */
void searchByRange(AVL& tree) {
	try {
		std::string startIp = "";
		std::string endIp = "";

		std::cout << "Enter start IP (e.g. 192.0.2.10): ";
		std::cin >> startIp;
		long long totalStartIp = getTotalIp(startIp);

		std::cout << "Enter end IP (e.g. 192.0.2.11): ";
		std::cin >> endIp;
		long long totalEndIp = getTotalIp(endIp);

		if (!validSearchRange(totalStartIp, totalEndIp)) {
			throw std::invalid_argument("Invalid IP range: " + startIp + " - " + endIp);
		}
		printRange(tree, startIp, endIp, std::cout);
	} catch (const std::invalid_argument& e) {
		std::cin.ignore(IGNORE_LIMIT, '\n');
		std::cerr << "Error: " << e.what() << '\n';
	}
}

/**
 * @brief Asks for n and prints the n most accessed IPs.
 * @param tree Tree that holds the IP statistics.
 * @note Time O(n * N) in the worst case, O(n) for a constant N. Space O(N).
 */
void showTopNodes(AVL& tree) {
	int n = 0;

	std::cout << "Enter n: ";
	if (!(std::cin >> n) || n <= 0) {
		std::cin.clear();
		std::cin.ignore(IGNORE_LIMIT, '\n');
		std::cout << "Invalid n\n";
	} else {
		printTopNodes(n, tree, std::cout);
	}
}

/**
 * @brief Executes the action that matches a menu option.
 * @param option Option chosen by the user.
 * @param tree Tree that holds the IP statistics.
 * @note Cost is that of the chosen action, see searchByRange and showTopNodes.
 */
void runOption(int option, AVL& tree) {
	if (option == RANGE_OPTION) {
		searchByRange(tree);
	} else if (option == TOP_OPTION) {
		showTopNodes(tree);
	} else if (option == EXIT_OPTION) {
		std::cout << "\nI'll be back\n";
	} else {
		std::cout << "Invalid option\n";
	}
}

/**
 * @brief Shows the menu repeatedly until the user chooses to exit.
 * @param tree Tree that holds the IP statistics.
 * @note Cost per iteration is that of the chosen action, see runOption.
 */
void runMenu(AVL& tree) {
	int option = INVALID_OPTION;

	do {
		printMenu();
		option = readOption();
		runOption(option, tree);
	} while (option != EXIT_OPTION);
}

/**
 * @brief Entry point: builds the tree from the log file given as argument,
 *        prints the final reports and starts the menu.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments, argv[1] is the log file path.
 * @return 0 on success, 1 if the arguments or the files are invalid.
 */
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: ./main <input_file>\n";
		return 1;
	}

	std::ofstream outFile(OUTPUT_FILE_NAME);

	if (!outFile.is_open()) {
		std::cout << "Couldn't create " << OUTPUT_FILE_NAME << "\n";
		return 1;
	}

	AVL* tree = getAVLTree(argv[1], outFile);

	if (tree == nullptr) {
		return 1;
	}
	printTopNodes(FINAL_TOP_COUNT, *tree, std::cout);
	printTopNodes(FINAL_TOP_COUNT, *tree, outFile);
	printTopIpBlock(*tree, std::cout);
	printTopIpBlock(*tree, outFile);
	runMenu(*tree);
	delete tree;
	return 0;
}