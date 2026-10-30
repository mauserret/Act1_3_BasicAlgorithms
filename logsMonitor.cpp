#include "AVL.h"
#include "Log.h"
#include <iostream>
#include <iomanip>
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

void printMonthlyReport(const std::string& month, AVL& tree, std::ostream& out) {
    int distinct = tree.getDistinctIps();
    double average = distinct == 0 ? 0.0
                   : static_cast<double>(tree.getTotalRecorded()) / distinct;
    out << "\n --- End of month " << month << " ---\n"
        << "Distinct IPs: " << distinct << "\n"
        << "Average accesses per IP: " << std::fixed << std::setprecision(2) << average << "\n"
        << "Top 5 IPs:\n";
    for (AVLNode* node : tree.nMostAccessed(5)) {
        out << "  " << node->strIp << " | Accesses: " << node->totalAccesses << "\n";
    }
}

// Prints the header, then the range query, to one stream
void printRange(AVL& tree, const std::string& startIp, const std::string& endIp, std::ostream& out) {
    out << "\n--- IPs in range: " << startIp << " - " << endIp << " ---\n";
    tree.rangeQuery(getTotalIp(startIp), getTotalIp(endIp), out);
}

// Finds the top IP and queries its whole x.y.z.0 - x.y.z.255 block
void printTopIpBlock(AVL& tree, std::ostream& out) {
    std::vector<AVLNode*> top = tree.nMostAccessed(1);
    if (top.empty()) return;

    std::string prefix = top[0]->strIp.substr(0, top[0]->strIp.rfind('.'));  // "x.y.z"
    out << "\nTop IP: " << top[0]->strIp << " -> querying its block";
    printRange(tree, prefix + ".0", prefix + ".255", out);
}

AVL* getAVLTree(const std::string& file_name, std::ofstream& outFile) {

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
            printMonthlyReport(previousMonth, *tree, std::cout);
            printMonthlyReport(previousMonth, *tree, outFile);
        }
        previousMonth = month;

        long long numerictIp = getTotalIp(ip);
        std::string address = ip.substr(0, ip.find(':'));   // drop the port
        int success = (msg.rfind("Accepted", 0) == 0) ? 1 : 0;   // starts-with check
        tree->insert(numerictIp, address, success);
    }

    // The last month never sees a "next month" line
    if (!previousMonth.empty()) {
        printMonthlyReport(previousMonth, *tree, std::cout);
        printMonthlyReport(previousMonth, *tree, outFile);
    }

    printTopNodes(15, *tree, std::cout);
    printTopNodes(15, *tree, outFile);

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

    std::ofstream outFile("output.txt");
    if (!outFile.is_open()) {
        std::cout << "Couldn't create output.txt\n";
        return 1;
    }

    AVL* Tree = getAVLTree(argv[1], outFile);

    printTopIpBlock(*Tree, std::cout);
    printTopIpBlock(*Tree, outFile);

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
                printRange(*Tree, startIp, endIp, std::cout);

            } catch (const std::invalid_argument& e) {
                std::cin.ignore(10000, '\n');
                std::cerr << "Error: " << e.what() << '\n';
            }
        } else if (option == 2) {
                try {
                    int n = 0;
                    std::cout << "Enter n: ";
                    if (!(std::cin >> n) || n <= 0) {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cout << "Invalid n\n";
                    } else {
                        printTopNodes(n, *Tree, std::cout);
                    }
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
