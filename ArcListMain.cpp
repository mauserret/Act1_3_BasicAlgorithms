/**
 * Reads bitacora.txt, builds an arc-list graph (IP -> server), ranks the
 * top 5 IPs by out-degree and the top 5 servers by in-degree, and writes the
 * report to the console and to output.txt.
 *
Authors: A01648241
         A01642638
         A01648221
         A01643073
Date: 19/11/2026
 */

#include "ArcList.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const std::string OUTPUT_FILE_NAME = "output.txt";
const int TOP_COUNT = 5;
const unsigned long long OCTET_BASE = 256;

struct RankEntry {
	int vertexId;
	int degree;
	long long accesses;
};

/**
 * @brief Turns a dotted IP into a number so IPs compare by value, not as text.
 * @param ip IP in the form a.b.c.d.
 * @return Numeric value of the IP.
 * @note Time O(L), L = length of the text. Space O(L).
 */
unsigned long long ipToNumber(const std::string& ip) {
	std::istringstream ipStream(ip);
	std::string octet = "";
	unsigned long long value = 0;
	while (std::getline(ipStream, octet, '.')) {
		value = value * OCTET_BASE + std::stoull(octet);
	}
	return value;
}

/**
 * @brief Tie-break by name: the smaller IP (numeric) or server name goes first.
 * @param first Name of the first vertex.
 * @param second Name of the second vertex.
 * @param isServer true if both are servers, false if both are IPs.
 * @return true if first must go before second.
 * @note Time O(L) for IPs and servers alike. Space O(L).
 */
bool nameComesFirst(const std::string& first, const std::string& second, bool isServer) {
	if (isServer) {
		return first < second;
	}
	return ipToNumber(first) < ipToNumber(second);
}

/**
 * @brief Ranking order: more degree, then more accesses, then smaller name.
 * @param first Entry to place.
 * @param second Entry to compare against.
 * @param graph Graph that owns the vertex names.
 * @param isServer true when ranking servers, false when ranking IPs.
 * @return true if first must go before second.
 * @note Time O(L), L = length of the names. Space O(L).
 */
bool ranksBefore(const RankEntry& first, const RankEntry& second, const ArcListGraph& graph, bool isServer) {
	if (first.degree != second.degree) {
		return first.degree > second.degree;
	}
	if (first.accesses != second.accesses) {
		return first.accesses > second.accesses;
	}
	return nameComesFirst(graph.name(first.vertexId), graph.name(second.vertexId), isServer);
}

/**
 * @brief Inserts an entry in the sorted top list and drops the last one if
 *        the list goes over TOP_COUNT.
 * @param ranking Top list, sorted from best to worst.
 * @param entry Candidate to insert.
 * @param graph Graph that owns the vertex names.
 * @param isServer true when ranking servers, false when ranking IPs.
 * @note Time O(TOP_COUNT) comparisons, so O(1) for a fixed top size.
 *       Space O(1).
 */
void insertInRanking(std::vector<RankEntry>& ranking, const RankEntry& entry, const ArcListGraph& graph, bool isServer) {
	int position = static_cast<int>(ranking.size());
	while (position > 0 && ranksBefore(entry, ranking[position - 1], graph, isServer)) {
		--position;
	}
	if (position < TOP_COUNT) {
		ranking.insert(ranking.begin() + position, entry);
	}
	if (static_cast<int>(ranking.size()) > TOP_COUNT) {
		ranking.pop_back();
	}
}

/**
 * @brief Builds the top ranking without std::sort: one pass over the
 *        vertices keeping a small sorted list.
 * @param graph Graph to rank.
 * @param isServer true for the in-degree ranking of servers, false for the
 *        out-degree ranking of IPs.
 * @return Up to TOP_COUNT entries, best first.
 * @note Time O(V + A): each degree and weight walks its own list once, and
 *       every insertion is O(1). Space O(1) besides the result.
 */
std::vector<RankEntry> buildRanking(const ArcListGraph& graph, bool isServer) {
	std::vector<RankEntry> ranking;
	for (int vertexId = 0; vertexId < graph.size(); ++vertexId) {
		if (graph.isServer(vertexId) == isServer) {
			RankEntry entry = {vertexId, 0, 0};
			entry.degree = isServer ? graph.inDegree(vertexId) : graph.outDegree(vertexId);
			entry.accesses = isServer ? graph.totalInWeight(vertexId) : graph.totalOutWeight(vertexId);
			insertInRanking(ranking, entry, graph, isServer);
		}
	}
	return ranking;
}

/**
 * @brief Reads the log and adds one access per line to the graph.
 * @param fileName Path of the log file.
 * @param graph Graph that receives the accesses.
 * @return true if the file was opened, false otherwise.
 * @note Time O(N * (V + D)): N lines, each one scans the vertex vector and an
 *       outgoing list of up to D arcs. Space O(1) besides the graph.
 */
bool loadGraph(const std::string& fileName, ArcListGraph& graph) {
	std::ifstream file(fileName);
	bool opened = file.is_open();
	std::string line = "";
	while (opened && std::getline(file, line)) {
		std::istringstream lineStream(line);
		std::string skipped = "";
		std::string source = "";
		std::string host = "";
		lineStream >> skipped >> skipped >> skipped >> source >> host;
		if (!host.empty()) {
			std::string ip = source.substr(0, source.find(':'));
			int originId = graph.getOrCreateVertexId(ip, false);
			int destinationId = graph.getOrCreateVertexId(host, true);
			graph.addAccess(originId, destinationId);
		}
	}
	return opened;
}

/**
 * @brief Prints the sizes of the graph: IPs, servers, arcs and accesses.
 * @param graph Graph to describe.
 * @param out Stream where the text is written.
 * @note Time O(V + A). Space O(1).
 */
void printGraphSize(const ArcListGraph& graph, std::ostream& out) {
	out << "Graph size\n";
	out << "Source IPs: " << graph.countVertices(false) << "\n";
	out << "Destination servers: " << graph.countVertices(true) << "\n";
	out << "Arcs: " << graph.arcCount() << "\n";
	out << "Total accesses: " << graph.totalWeight() << "\n\n";
}

/**
 * @brief Prints one ranking as a table.
 * @param title Heading of the table.
 * @param nameLabel Label of the name column (IP or Server).
 * @param degreeLabel Label of the degree column (Out-degree or In-degree).
 * @param ranking Entries to print, best first.
 * @param graph Graph that owns the vertex names.
 * @param out Stream where the text is written.
 * @note Time O(TOP_COUNT). Space O(1).
 */
void printRanking(const std::string& title, const std::string& nameLabel, const std::string& degreeLabel,
	const std::vector<RankEntry>& ranking, const ArcListGraph& graph, std::ostream& out) {
	out << title << "\n";
	out << std::left << std::setw(34) << nameLabel << std::setw(12) << degreeLabel << "Accesses\n";
	for (const RankEntry& entry : ranking) {
		out << std::left << std::setw(34) << graph.name(entry.vertexId) << std::setw(12) << entry.degree << entry.accesses << "\n";
	}
	out << "\n";
}

/**
 * @brief Prints how many arcs are visited to list the IPs that reach a
 *        server: with the incoming list and with only outgoing lists.
 * @param serverId Id of the server to look up.
 * @param graph Graph that owns the arcs.
 * @param out Stream where the text is written.
 * @note Time O(inDegree) to count the incoming list. Space O(1).
 */
void printSourceLookupCost(int serverId, const ArcListGraph& graph, std::ostream& out) {
	out << "Arcs visited to list the IPs that reach " << graph.name(serverId) << "\n";
	out << "With incoming list: " << graph.inDegree(serverId) << "\n";
	out << "With only outgoing lists: " << graph.arcCount() << "\n";
}

/**
 * @brief Prints the full report: size, both rankings and the lookup cost.
 * @param graph Graph to report.
 * @param ipRanking Top IPs by out-degree.
 * @param serverRanking Top servers by in-degree.
 * @param out Stream where the text is written.
 * @note Time O(V + A). Space O(1).
 */
void printReport(const ArcListGraph& graph, const std::vector<RankEntry>& ipRanking,
	const std::vector<RankEntry>& serverRanking, std::ostream& out) {
	printGraphSize(graph, out);
	printRanking("Out-degree ranking", "IP", "Out-degree", ipRanking, graph, out);
	printRanking("In-degree ranking", "Server", "In-degree", serverRanking, graph, out);
	if (!serverRanking.empty()) {
		printSourceLookupCost(serverRanking[0].vertexId, graph, out);
	}
}

/**
 * @brief Entry point: loads the log, ranks the nodes and prints the report.
 * @param argc Number of command-line arguments.
 * @param argv Arguments; argv[1] is the path of the log file.
 * @return 0 on success, 1 if the arguments or files are wrong.
 * @note Time O(N * (V + D)), dominated by loadGraph. Space O(V + A).
 */
int main(int argc, char* argv[]) {
	int exitCode = 0;
	if (argc < 2) {
		std::cout << "Usage: ./main <input_file>\n";
		exitCode = 1;
	} else {
		ArcListGraph graph;
		std::ofstream outFile(OUTPUT_FILE_NAME);
		if (!loadGraph(argv[1], graph)) {
			std::cout << "Couldn't open the file\n";
			exitCode = 1;
		} else if (!outFile.is_open()) {
			std::cout << "Couldn't create " << OUTPUT_FILE_NAME << "\n";
			exitCode = 1;
		} else {
			std::vector<RankEntry> ipRanking = buildRanking(graph, false);
			std::vector<RankEntry> serverRanking = buildRanking(graph, true);
			printReport(graph, ipRanking, serverRanking, std::cout);
			printReport(graph, ipRanking, serverRanking, outFile);
		}
	}
	return exitCode;
}