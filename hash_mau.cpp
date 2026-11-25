#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

struct ServerInfo {
    int attempts = 0;
    std::unordered_map<std::string, int> ips;
    std::array<int, 24> hourlyAccess{};
};

void sortMap(std::unordered_map<std::string, ServerInfo>& servers, 
             std::vector<std::pair<std::string, ServerInfo>>& sortedServers) {
                for (const auto& entry : servers) {
                    sortedServers.push_back(entry);
                }
                // sort with std::sort using a lambda function to act on number of attempts (ascending)
                std::sort(sortedServers.begin(), sortedServers.end(), [](const auto& a, const auto& b) {
                    return a.second.attempts > b.second.attempts;
                }); 
}

bool getServers(const std::string& fileName, std::unordered_map<std::string, ServerInfo>& serversMap) {
    std::ifstream file(fileName);
    bool opened = file.is_open();
    std::string line = "";

    while (opened && std::getline(file, line)){
        std::istringstream lineStream(line);
        // Oct 24 03:27:38 198.51.100.66:5062 ftp.almacen-norte.example Failed password for admin
        std::string skipped = "";
        std::string timeStr = "";
        std::string source = "";
        std::string server = "";
        lineStream >> skipped >> skipped >> timeStr >> source >> server;

        std::string ip = source.substr(0, source.find(':'));
        std::string hourStr = timeStr.substr(0, timeStr.find(':'));
        int hour = std::stoi(hourStr);
        

        ServerInfo& s = serversMap[server];
        s.attempts++;
        s.ips[ip]++;
        s.hourlyAccess[hour]++;
    }
    return opened;
}

void printServers(const std::vector<std::pair<std::string, ServerInfo>>& sortedServers) {
    std::cout << std::fixed << std::setprecision(2); // 2 decimal places
    for (const auto& entry : sortedServers) {
        const ServerInfo& info = entry.second;
        double average = static_cast<double>(info.attempts) / info.ips.size();
        std::cout << std::left << std::setw(32) << entry.first
                << std::right << std::setw(10) << info.attempts
                << std::setw(10) << info.ips.size()
                << std::setw(10) << average << "\n";
    }
}

int main(int argc, char* argv[]) {
    int exitCode = 0;
	if (argc < 2) {
		std::cout << "Usage: ./main <input_file>\n";
		exitCode = 1;
	} else {
		std::unordered_map<std::string, ServerInfo> servers;
        std::vector<std::pair<std::string, ServerInfo>> sortedServers;
		if (!getServers(argv[1], servers)) {
			std::cout << "Couldn't open the file\n";
			exitCode = 1;
        /*
		} else if (!outFile.is_open()) {
			std::cout << "Couldn't create " << OUTPUT_FILE_NAME << "\n";
			exitCode = 1;
        */
		} else {
			sortMap(servers, sortedServers);
            printServers(sortedServers);
		}
	}
	return exitCode;
    
}