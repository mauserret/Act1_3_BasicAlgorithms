#include "AVL.h"
#include "Log.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <stdexcept>

int checkOctet(const std::string&ip) {
    try
    {
        size_t octetSize = 0;
        int ipOctet = stoi(ip, &octetSize);
        if (octetSize != ip.size()) {
            throw std::invalid_argument("Invalid octet IP: " + ip);
        }
        if (ipOctet < 0 || ipOctet > 255) {
            throw std::invalid_argument("Invalid octet IP: " + ip);
        } 
        return ipOctet;
    }
    catch(const std::exception&)
    {
        throw std::invalid_argument("Invalid octet IP: " + ip);
    }
    
}

long long getTotalIp (const std::string& ip) {

    std::string strIp1 = "";
    std::string strIp2 = "";
    std:: string strIp3 = "";
    std::string strIp4 = "";
    int ip1 = 0;
    int ip2 = 0;
    int ip3 = 0;
    int ip4 = 0;

    std::stringstream ss(ip);
    getline(ss, strIp1, '.');
    getline(ss, strIp2, '.');
    getline(ss, strIp3, '.');
    getline(ss, strIp4, ':');

    ip1 = checkOctet(strIp1);
    ip2 = checkOctet(strIp2);
    ip3 = checkOctet(strIp3);
    ip4 = checkOctet(strIp4);

    return ip1 * pow(256, 3) + ip2 * pow(256, 2) + ip3 * 256 + ip4;
}

void printTopNodes(const int& n, AVL& tree) {
    std::cout << "\n --- Top " << n << " IPs ---\n";
    for (AVLNode* node : tree.nMostAccessed(n)) {
        float totAccess = node->totalAccesses;
        float totSuccess = node->totalSuccess;
    
        std::cout << node->strIp 
                  << " | Accesses: " << totAccess
                  << " | Success: " << totSuccess
                  << " | Success %: " << float((totSuccess / totAccess) * 100) <<  "\n";
    }
}

void printMonthlyReport(std::string& month, AVL&tree) {
    int distinct = tree.getDistinctIps();
    double average = distinct == 0 ? 0.0 : 
                    static_cast<double>(tree.getTotalRecorded()) / distinct;
    std::cout << "\n --- End of month " << month << "---\n"
              << "Distinct IPs: " << distinct << "\n"
              << "Average accesses per IP: " << average << "\n"
              << "Top 5 IPs: \n";
    for (AVLNode* node : tree.nMostAccessed(5)) {
        std::cout << "  " << node->strIp 
                  << " | Accesses: " << node->totalAccesses << "\n";
    }
}

AVL* getAVLTree(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cout << "Couldn't open the file \n";
        exit(1);
    }

    AVL* tree = new AVL;
    std::string previousMonth = "";

    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string month = "", strDay = "", strHour = "", strMin = "", strSec = "", ip = "", host = "", msg = "";


        getline(ss, month, ' ');
        getline(ss, strDay, ' ');
        getline(ss, strHour, ':');
        getline(ss, strMin, ':');
        getline(ss, strSec, ' ');
        getline(ss, ip, ' ');
        getline(ss, host, ' ');
        getline(ss, msg);

        if (!previousMonth.empty() && month != previousMonth) {
            printMonthlyReport(previousMonth, *tree);
        }
        previousMonth = month;

        long long numerictIp = getTotalIp(ip);
        std::string address = ip.substr(0, ip.find(':'));   // drop the port
        int success = (msg.rfind("Accepted", 0) == 0) ? 1 : 0;   // starts-with check
        tree->insert(numerictIp, address, success);
    }

    // The last month never sees a "next month" line
    if (!previousMonth.empty()) {
        printMonthlyReport(previousMonth, *tree);
    }

    printTopNodes(15, *tree);

    file.close();
    return tree;
}

template <typename T>
bool validSearchRange(const T& startRange, const T& endRange) {
    return endRange >= startRange;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./main <input_file>\n";
        return 1;
    }

    AVL* Tree = getAVLTree(argv[1]);

    int option = 0;
    do {
        std::cout << "\nSelect an option: " << "\n"
        << "1. Search by IP range" << "\n"
        << "2. Get top n accessed IPs" << "\n"
        << "3. Exit\n" 
        << "Option: ";

        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid option\n";
            continue;
        }
        if (option == 1) {
            try {
                std::string startIp = "", endIp = "";
                
                std::cout << "Enter start IP (Ej. 192.0.2.10): ";
                std::cin >> startIp;

                long long totalStartIp = getTotalIp(startIp);

                std::cout << "Enter end IP (Ej. 192.0.2.11): ";
                std::cin >> endIp;

                long long totalEndIp = getTotalIp(endIp);

                if (!validSearchRange(totalStartIp, totalEndIp)) {
                    throw std::invalid_argument("Invalid IP range: " + startIp + " - " + endIp);
                }
                std::cout << "\n--- IPs in range: " << startIp << " - " << endIp << "---\n";
                Tree->rangeQuery(totalStartIp, totalEndIp); 

            } catch (const std::invalid_argument& e) {
                std::cin.ignore(10000, '\n');
                std::cerr << "Error: " << e.what() << '\n';
            }
        } else if (option == 2) {
                try {
                    int n;
                    std::cout << "Enter n: ";
                    std::cin >> n;
                    printTopNodes(n, *Tree);
                } catch (const std::invalid_argument& e) {
                    std::cin.ignore(10000, '\n');
                    std::cerr << "Error: " << e.what() << '\n';
                }
        } else if(option == 3) {
            std::cout << "\nI'll be back\n";
        } else {
            std::cout << "Invalid option\n";
        }
    } while(option != 3);

    delete Tree;
    return 0;    
}
