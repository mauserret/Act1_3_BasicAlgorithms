/*
 * Reads a log file to build an access graph, computes graph size metrics,
 * and prints top rankings for source IPs and destination servers.
 * Author: A00000000
 * Date: 2026-11-19
 */

#include "ArcMoon.h"
#include "IpUtils.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

const std::size_t TOP_LIMIT = 5;
const int RANK_COLUMN_WIDTH = 5;
const int NAME_COLUMN_WIDTH = 24;
const int DEGREE_COLUMN_WIDTH = 14;
const int ACCESS_COLUMN_WIDTH = 16;
const int SEPARATOR_WIDTH = 59;
const std::string DEFAULT_LOG_FILE = "bitacora.txt";

struct GraphSummary {
	int sourceIps = 0;
	int destinationServers = 0;
	long long arcs = 0;
	long long totalAccesses = 0;
};

struct RankEntry {
	int vertexId = 0;
	std::string name = "";
	bool isServer = false;
	int degree = 0;
	long long totalWeight = 0;
};

/*
 * Removes the port suffix from an IP address string if present.
 * @param ipPort Address string formatted as IP:port or plain IP.
 * @return IP address without the port part.
 */
std::string stripPort(const std::string& ipPort) {
	std::size_t colonPosition = ipPort.find(':');
	std::string result = ipPort;

	if (colonPosition != std::string::npos) {
		result = ipPort.substr(0, colonPosition);
	}
	return result;
}

/*
 * Computes summary counts of source IPs, destination servers, arcs, and accesses.
 * @param graph Graph containing IP and server vertices and access weights.
 * @return Populated GraphSummary structure with the computed metrics.
 */
GraphSummary computeGraphSummary(const ArcListGraph& graph) {
	GraphSummary summary;
	int vertexCount = graph.size();

	for (int i = 0; i < vertexCount; ++i) {
		if (graph.isServer(i)) {
			summary.destinationServers += 1;
		} else {
			summary.sourceIps += 1;
			summary.arcs += graph.outDegree(i);
			summary.totalAccesses += graph.totalOutWeight(i);
		}
	}
	return summary;
}

/*
 * Prints the graph size summary metrics to the given output stream.
 * @param summary Computed graph summary metrics.
 * @param out Destination output stream.
 * @return Nothing.
 */
void printGraphSummary(const GraphSummary& summary, std::ostream& out) {
	out << "\n=== Graph Size ===\n";
	out << "Source IPs:          " << summary.sourceIps << "\n";
	out << "Destination Servers: " << summary.destinationServers << "\n";
	out << "Arcs:                " << summary.arcs << "\n";
	out << "Total Accesses:      " << summary.totalAccesses << "\n";
}

/*
 * Determines whether entry a should rank ahead of entry b using degree,
 * total accesses, and numeric IP or lexicographical name tie-breakers.
 * @param first First candidate entry to compare.
 * @param second Second candidate entry to compare.
 * @return True if first belongs before second in the ranking, false otherwise.
 */
bool comesBefore(const RankEntry& first, const RankEntry& second) {
	bool decision = false;

	if (first.degree != second.degree) {
		decision = first.degree > second.degree;
	} else if (first.totalWeight != second.totalWeight) {
		decision = first.totalWeight > second.totalWeight;
	} else if (!first.isServer && !second.isServer) {
		try {
			long long firstIpValue = getTotalIp(first.name);
			long long secondIpValue = getTotalIp(second.name);

			if (firstIpValue != secondIpValue) {
				decision = firstIpValue < secondIpValue;
			} else {
				decision = first.name < second.name;
			}
		} catch (const std::invalid_argument&) {
			decision = first.name < second.name;
		}
	} else {
		decision = first.name < second.name;
	}
	return decision;
}

/*
 * Sorts a list of ranking entries in place using insertion sort and comesBefore.
 * @param entries Vector of ranking entries to sort.
 * @return Nothing.
 */
void sortRanking(std::vector<RankEntry>& entries) {
	std::size_t count = entries.size();

	for (std::size_t i = 1; i < count; ++i) {
		RankEntry current = entries[i];
		std::size_t position = i;
		bool shouldShift = position > 0 && comesBefore(current, entries[position - 1]);

		while (shouldShift) {
			entries[position] = entries[position - 1];
			position -= 1;
			shouldShift = position > 0 && comesBefore(current, entries[position - 1]);
		}
		entries[position] = current;
	}
}

/*
 * Builds the ranked list of top source IPs ordered by out-degree and tie-breakers.
 * @param graph Source access graph.
 * @param limit Maximum number of entries to keep in the top list.
 * @return Vector containing up to limit top source IP entries.
 */
std::vector<RankEntry> buildTopIps(const ArcListGraph& graph, std::size_t limit) {
	std::vector<RankEntry> entries;
	int vertexCount = graph.size();

	for (int i = 0; i < vertexCount; ++i) {
		if (!graph.isServer(i)) {
			RankEntry entry;

			entry.vertexId = i;
			entry.name = graph.name(i);
			entry.isServer = false;
			entry.degree = graph.outDegree(i);
			entry.totalWeight = graph.totalOutWeight(i);
			entries.push_back(entry);
		}
	}
	sortRanking(entries);
	if (entries.size() > limit) {
		entries.resize(limit);
	}
	return entries;
}

/*
 * Builds the ranked list of top destination servers ordered by in-degree and tie-breakers.
 * @param graph Source access graph.
 * @param limit Maximum number of entries to keep in the top list.
 * @return Vector containing up to limit top server entries.
 */
std::vector<RankEntry> buildTopServers(const ArcListGraph& graph, std::size_t limit) {
	std::vector<RankEntry> entries;
	int vertexCount = graph.size();

	for (int i = 0; i < vertexCount; ++i) {
		if (graph.isServer(i)) {
			RankEntry entry;

			entry.vertexId = i;
			entry.name = graph.name(i);
			entry.isServer = true;
			entry.degree = graph.inDegree(i);
			entry.totalWeight = graph.totalInWeight(i);
			entries.push_back(entry);
		}
	}
	sortRanking(entries);
	if (entries.size() > limit) {
		entries.resize(limit);
	}
	return entries;
}

/*
 * Prints a formatted ranking table for a given list of entries.
 * @param title Heading displayed above the table.
 * @param columnName Header label for the entity column.
 * @param degreeLabel Header label for the degree column.
 * @param ranking Ordered vector of ranking entries to display.
 * @return Nothing.
 */
void printRankingTable(const std::string& title,
                       const std::string& columnName,
                       const std::string& degreeLabel,
                       const std::vector<RankEntry>& ranking) {
	std::cout << "\n=== " << title << " ===\n";
	std::cout << std::left
	          << std::setw(RANK_COLUMN_WIDTH) << "#"
	          << std::setw(NAME_COLUMN_WIDTH) << columnName
	          << std::setw(DEGREE_COLUMN_WIDTH) << degreeLabel
	          << std::setw(ACCESS_COLUMN_WIDTH) << "Total Accesos" << "\n";
	std::cout << std::string(SEPARATOR_WIDTH, '-') << "\n";

	std::size_t rankCount = ranking.size();

	for (std::size_t i = 0; i < rankCount; ++i) {
		std::cout << std::left
		          << std::setw(RANK_COLUMN_WIDTH) << (i + 1)
		          << std::setw(NAME_COLUMN_WIDTH) << ranking[i].name
		          << std::setw(DEGREE_COLUMN_WIDTH) << ranking[i].degree
		          << std::setw(ACCESS_COLUMN_WIDTH) << ranking[i].totalWeight << "\n";
	}
}

/*
 * Parses a single log line and records the access edge into the graph if valid.
 * @param line Raw text line from the log file.
 * @param graph Target graph where vertices and accesses are recorded.
 * @return Nothing.
 */
void processLogLine(const std::string& line, ArcListGraph& graph) {
	if (!line.empty()) {
		std::istringstream lineStream(line);
		std::string month = "";
		std::string day = "";
		std::string timeStr = "";
		std::string ipPort = "";
		std::string server = "";

		if (lineStream >> month >> day >> timeStr >> ipPort >> server) {
			std::string ip = stripPort(ipPort);

			if (!ip.empty() && !server.empty()) {
				int sourceId = graph.getId(ip, false);
				int serverId = graph.getId(server, true);

				graph.addAccess(sourceId, serverId);
			}
		}
	}
}

/*
 * Program entry point: opens the log file, populates the graph, and prints reports.
 * @param argc Number of command-line arguments passed to the program.
 * @param argv Array of command-line argument strings.
 * @return 0 on successful execution, or 1 if the input file cannot be opened.
 */
int main(int argc, char* argv[]) {
	std::string fileName = DEFAULT_LOG_FILE;

	if (argc >= 2) {
		fileName = argv[1];
	}

	std::ifstream inputFile(fileName);
	int exitCode = 0;

	if (!inputFile.is_open()) {
		std::cerr << "Error: no se pudo abrir " << fileName << "\n";
		exitCode = 1;
	} else {
		ArcListGraph graph;
		std::string line = "";

		while (std::getline(inputFile, line)) {
			processLogLine(line, graph);
		}

		GraphSummary summary = computeGraphSummary(graph);

		printGraphSummary(summary, std::cout);

		std::vector<RankEntry> topIps = buildTopIps(graph, TOP_LIMIT);
		std::vector<RankEntry> topServers = buildTopServers(graph, TOP_LIMIT);

		printRankingTable("Top 5 IPs por Out-Degree", "IP", "Out-Degree", topIps);
		printRankingTable("Top 5 Servidores por In-Degree", "Servidor", "In-Degree", topServers);
	}
	return exitCode;
}