/**
 * @file hashMapServers.cpp
 * @brief Network monitor that reads a log of access attempts and reports, for each
 * server, its attempts, different IPs, average attempts per IP and, for the
 * suspicious servers, its attempts per hour of the day.
 * @author Student ID(s): A01648241, A01642638, A01648221, A01643073
 * @date Modified: 2026-11-27
 */

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

const int HOURS_PER_DAY = 24;
const int NAME_WIDTH = 32;
const int NUMBER_WIDTH = 10;
const int AVERAGE_DECIMALS = 1;
const std::array<std::string, 3> SUSPICIOUS_SERVERS = {
	"db.nube-sur.example",
	"print.almacen-norte.example",
	"intranet.dev-interno.example"
};

// Data gathered for one server
struct ServerInfo {
	int attempts = 0;
	std::unordered_map<std::string, int> attemptsByIp{};
	std::array<int, HOURS_PER_DAY> hourlyAttempts{};
};

// Fields of one log record that the monitor needs
struct LogRecord {
	std::string ip = "";
	std::string server = "";
	int hour = 0;
};

using ServerTable = std::unordered_map<std::string, ServerInfo>;
using ServerEntry = ServerTable::value_type;

/**
 * @brief Extract the source IP, server name and hour from one log line.
 * Expected format: month day hh:mm:ss ip:port server message.
 * @param line Text of the log line.
 * @param record Output record where the extracted fields are stored.
 * @return true if the line has all the fields and a valid hour, false otherwise.
 * @note Time O(L), where L is the length of the line. Space O(L).
 */
bool parseRecord(const std::string& line, LogRecord& record) {
	std::istringstream lineStream(line);
	std::string skippedField = "";
	std::string source = "";

	// Reading the hour as a number leaves ":mm:ss" as the next field to skip
	bool valid = static_cast<bool>(lineStream >> skippedField >> skippedField >> record.hour
		>> skippedField >> source >> record.server);
	valid = valid && record.hour >= 0 && record.hour < HOURS_PER_DAY;

	if (valid) {
		record.ip = source.substr(0, source.find(':'));
	}
	return valid;
}

/**
 * @brief Add one access attempt to the data of its server.
 * @param record Record with the IP, server name and hour of the attempt.
 * @param servers Table of servers to update.
 * @return None.
 * @note Time O(L) on average, where L is the length of the names, and
 * O((S + I) * L) in the worst case due to collisions or rehashing, where S is the
 * number of servers and I the number of IPs of the server. Space O(L).
 */
void registerAttempt(const LogRecord& record, ServerTable& servers) {
	ServerInfo& info = servers[record.server];
	info.attempts++;
	// operator[] checks if the IP exists and inserts it with 0 attempts if it does not
	info.attemptsByIp[record.ip]++;
	info.hourlyAttempts[record.hour]++;
}

/**
 * @brief Read the log file one record at a time and fill the table of servers.
 * Lines that do not have the expected format are ignored.
 * @param fileName Path of the log file.
 * @param servers Table of servers to fill.
 * @return true if the file could be opened, false otherwise.
 * @note Time O(N * L) on average, where N is the number of records and L the
 * maximum line length. Space O((S + P) * L), where S is the number of servers
 * and P the number of different server and IP pairs.
 */
bool loadServers(const std::string& fileName, ServerTable& servers) {
	std::ifstream file(fileName);
	bool opened = file.is_open();
	std::string line = "";

	while (opened && std::getline(file, line)) {
		LogRecord record{};
		if (parseRecord(line, record)) {
			registerAttempt(record, servers);
		}
	}
	return opened;
}

/**
 * @brief Order two servers by attempts (descending). Ties are broken by name.
 * @param first Entry of the first server.
 * @param second Entry of the second server.
 * @return true if first must go before second, false otherwise.
 * @note Time O(L), where L is the length of the names. Space O(1).
 */
bool goesBefore(const ServerEntry* first, const ServerEntry* second) {
	bool result = first->second.attempts > second->second.attempts;

	if (first->second.attempts == second->second.attempts) {
		result = first->first < second->first;
	}
	return result;
}

/**
 * @brief Build a list of the servers sorted from most to fewest attempts.
 * The list holds pointers so the data of each server is not copied.
 * @param servers Table of servers.
 * @return Vector of pointers to the table entries, in the requested order.
 * @note Time O(S * log S * L), where S is the number of servers and L the length
 * of the names. Space O(S).
 */
std::vector<const ServerEntry*> sortServersByAttempts(const ServerTable& servers) {
	std::vector<const ServerEntry*> sortedServers{};
	sortedServers.reserve(servers.size());

	for (const ServerEntry& entry : servers) {
		sortedServers.push_back(&entry);
	}

	std::sort(sortedServers.begin(), sortedServers.end(), goesBefore);
	return sortedServers;
}

/**
 * @brief Print one line per server with its attempts, different IPs and average
 * attempts per IP.
 * @param sortedServers Servers in the order in which they must be printed.
 * @return None.
 * @note Time O(S * L), where S is the number of servers and L the length of the
 * names. Space O(1).
 */
void printServers(const std::vector<const ServerEntry*>& sortedServers) {
	std::cout << std::left << std::setw(NAME_WIDTH) << "Server"
		<< std::right << std::setw(NUMBER_WIDTH) << "Attempts"
		<< std::setw(NUMBER_WIDTH) << "IPs"
		<< std::setw(NUMBER_WIDTH) << "Average" << "\n";
	std::cout << std::fixed << std::setprecision(AVERAGE_DECIMALS);

	for (const ServerEntry* entry : sortedServers) {
		const ServerInfo& info = entry->second;
		double average = static_cast<double>(info.attempts) / info.attemptsByIp.size();
		std::cout << std::left << std::setw(NAME_WIDTH) << entry->first
			<< std::right << std::setw(NUMBER_WIDTH) << info.attempts
			<< std::setw(NUMBER_WIDTH) << info.attemptsByIp.size()
			<< std::setw(NUMBER_WIDTH) << average << "\n";
	}
}

/**
 * @brief Print the attempts that a server received in each hour of the day.
 * @param name Name of the server.
 * @param info Data of the server.
 * @return None.
 * @note Time O(L + H), where L is the length of the name and H the hours per day
 * (constant, 24). Space O(1).
 */
void printHourlyAttempts(const std::string& name, const ServerInfo& info) {
	std::cout << "\nAttempts per hour of " << name << "\n";

	for (int hour = 0; hour < HOURS_PER_DAY; hour++) {
		std::cout << std::setw(2) << hour << ":00"
			<< std::setw(NUMBER_WIDTH) << info.hourlyAttempts[hour] << "\n";
	}
}

/**
 * @brief Print the attempts per hour of every suspicious server.
 * @param servers Table of servers.
 * @return None.
 * @note Time O(M * (L + H)) on average, where M is the number of suspicious
 * servers, L the length of the names and H the hours per day. Space O(1).
 */
void printSuspiciousServers(const ServerTable& servers) {
	for (const std::string& name : SUSPICIOUS_SERVERS) {
		ServerTable::const_iterator entry = servers.find(name);
		if (entry != servers.end()) {
			printHourlyAttempts(name, entry->second);
		} else {
			std::cout << "\nServer not found in the log: " << name << "\n";
		}
	}
}

/**
 * @brief Load the log file given as argument and print the monitor report.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments; argv[1] is the path of the log file.
 * @return 0 if the report was printed, 1 if the argument is missing or the file
 * could not be opened.
 * @note Time O(N * L + S * log S * L) on average, where N is the number of
 * records, S the number of servers and L the maximum line length.
 * Space O((S + P) * L), where P is the number of different server and IP pairs.
 */
int main(int argc, char* argv[]) {
	int exitCode = 0;

	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <input_file>\n";
		exitCode = 1;
	} else {
		ServerTable servers{};
		if (!loadServers(argv[1], servers)) {
			std::cout << "Couldn't open the file\n";
			exitCode = 1;
		} else {
			printServers(sortServersByAttempts(servers));
			printSuspiciousServers(servers);
		}
	}
	return exitCode;
}