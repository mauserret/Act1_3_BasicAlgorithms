/* This program reads the data set called "bitacora.txt" that contains different logs, saves it in a vector with pointers to class Log, then
it orders the logs from the earliest to the latest date and prompts the user for a date range (e.g. Sep 10 to Sep 15). The program will determine
if there are any logs for the given range, prints them into the console, and creates an output file named "orderedLogs.txt" containing the logs inside
the provided range.
// Compilation string (masaru): g++ main.cpp -o main.exe; ./main.exe bitacora.txt  

Authors: A01648241
         A01642638
         A01648221
         A01643073
Date: 11/09/2026
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cmath>

/**
 * @class Log
 * @brief Represents a single server log entry containing timestamps, ip address, host, and message.
 */
class Log {
private:
    std::string month = "";
    double doubleMonth = 0;
    double day = 0;
    double hour = 0;
    double min = 0;
    double sec = 0;
    double totalTime = 0;
    long long numericIp = 0;
    std::string ip = "";
    std::string host = "";
    std::string message = "";

public:

    Log(std::string month, double doubleMonth, double day, double hour, double min, double sec, double totalTime,
        std::string ip, long long numericIp, std::string host, std::string message) :
        month{ month }, doubleMonth{ doubleMonth }, day{ day }, hour{ hour }, min{ min }, sec{ sec }, totalTime{ totalTime },
        ip{ ip }, numericIp{ numericIp }, host{ host }, message{ message } {
    }

    /**
     * @brief Overloads the stream insertion operator to format the output for the log entry.
     * @param os Output stream reference where the output will be printed.
     * @param log The log to format its output.
     * @return std::ostream& reference to the modified output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Log& log) {
        os << log.getMonth() << " "
            << std::setfill('0') << std::setw(2) << static_cast<int>(log.getDay()) << " "
            << std::setfill('0') << std::setw(2) << static_cast<int>(log.getHour()) << ":"
            << std::setfill('0') << std::setw(2) << static_cast<int>(log.getMin()) << ":"
            << std::setfill('0') << std::setw(2) << static_cast<int>(log.getSec()) << " "
            << log.getIp() << " "
            << log.getHost() << " "
            << log.getMessage() << " "
            << "\n";
        return os;
    }

    std::string getMonth() const { return month; }
    double getDay() const { return day; }
    double getHour() const { return hour; }
    double getMin() const { return min; }
    double getSec() const { return sec; }
    double getTotalTime() const { return totalTime; }
    std::string getIp() const { return ip; }
    long long getNumericIp() const {return numericIp; }
    std::string getHost() const { return host; }
    std::string getMessage() const { return message; }
};

enum class Criteria {
    DATE, 
    IP,
};
/**
 * @brief Sorts a vector with the merge sort algorithm by using recursion to divide and conquer; uses the merge() function.
 *
 * @param v Reference to the vector to sort.
 * @param l Starting index of the range to sort.
 * @param r Ending index of the range to sort.
 */
template <typename T>
void mergeSort(std::vector<T>& v, int l, int r, Criteria sortingCriteria) {
    if (l >= r) return;
    int mid = l + (r - l) / 2;
    mergeSort(v, l, mid, sortingCriteria);
    mergeSort(v, mid + 1, r, sortingCriteria);
    merge(v, l, mid, r, sortingCriteria);
}

/**
 * @brief Merges two sorted subarrays into a sorted subarray. Helper function for mergeSort()
 *
 * @param v Reference to the vector with elements to merge.
 * @param l Starting index of the left subarray.
 * @param mid Ending index of the left subarray
 * @param r Ending index of the right subarray.
 */
template <typename T>
void merge(std::vector<T>& v, int l, int mid, int r, Criteria sortingCriteria) {
    std::vector<T> temp(r - l + 1);
    int i = l;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= r) {
        bool condition = false;
        switch (sortingCriteria) {
            case Criteria::DATE : {
                condition = (v[i]->getTotalTime() <= v[j]->getTotalTime());
                break;
            }
            case Criteria::IP : {
                condition = (v[i]->getNumericIp() <= v[j]->getNumericIp());
                break;
            }
        }

        if (condition) {
            temp[k++] = v[i++];
        }
        else {
            temp[k++] = v[j++];
        }
    }
    while (i <= mid)
        temp[k++] = v[i++];

    while (j <= r)
        temp[k++] = v[j++];

    for (int x = 0; x < r - l + 1; x++)
        v[l + x] = temp[x];
}

/**
 * @brief Saves the ordered vector of logs into the created file named orderedLogs
 * @param v The vector to save into the file, Logs vector.
 */
template <typename T>
void saveSortedLogs(const std::vector<T>& v, const std::string outputFileName) {
    std::ofstream orderedFile(outputFileName);
    if (orderedFile.is_open()) {
        for (Log* log : v) {
            orderedFile << *log;
        }
    } else {
        std::cout << "Couldn't create the file \n";
    }
    
}

/**
 * @brief Searches for the first and last log inside a given time range using binary search. 
 * 
 * @param v Reference to the vector containing the sorted logs. 
 * @param start Beginning of the desired time range. 
 * @param end End of the desired time range. 
 * @return std::pair<int, int> Pair containing the first and last indexes found. 
 */
template <typename T>
std::pair<int, int> binarySearch(const std::vector<T>& v, double start, double end, Criteria searchCriteria) {
    int startIndex = -1;
    int endIndex = -1;

    int l = 0;
    int r = v.size() - 1;
    while (l <= r) {
        bool condition = false;
        int m = l + (r - l) / 2;
        switch (searchCriteria) {
            case Criteria::DATE : {
                condition = (v[m]->getTotalTime() >= start);
                break;
            }
            case Criteria::IP : {
                condition = (v[m]->getNumericIp() >= start);
                break;
            }
        }
        if (condition) {
            startIndex = m;
            r = m - 1;
        }
        else {
            l = m + 1;
        }
    }

    l = 0;
    r = v.size() - 1;

    while (l <= r) {
        bool condition = 0;
        int m = l + (r - l) / 2;
        switch (searchCriteria) {
            case Criteria::DATE : {
                condition = (v[m]->getTotalTime() <= end);
                break;
            }
            case Criteria::IP : {
                condition = (v[m]->getNumericIp() <= end);
                break;
            }
        }
        if (condition) {
            endIndex = m;
            l = m + 1;
        }
        else {
            r = m - 1;
        }
    }

    if (startIndex == -1 || endIndex == -1 || startIndex > endIndex) {
        return { -1, -1 };
    }
    return { startIndex, endIndex };
}

/** 
* @brief Converts a month abbreviation into a numeric value. 
* 
* @param m Reference to the string containing the month abbreviation. 
* @return int Numeric value associated with the month. 
*/
int monthToInt(std::string& m) {
    std::unordered_map<std::string, int> convert{
        {"Jan", 0},
        {"Feb", 31},
        {"Mar", 59},
        {"Apr", 90},
        {"May", 120},
        {"Jun", 151},
        {"Jul", 181},
        {"Aug", 212},
        {"Sep", 243},
        {"Oct", 273},
        {"Nov", 304},
        {"Dec", 334},
    };
    return convert[m];
}

/** 
* @brief Calculates a numeric value representing the total time of a log. 
*
* @param month Numeric value of the month. 
* @param day Day of the month. 
* @param hour Hour of the log. 
* @param min Minute of the log. 
* @param sec Second of the log. 
* @return double Total time converted into hours. 
*/
double getTotalTime(const double& month, const double& day, const double& hour, const double& min, const double& sec) {
    return (month + day) * 24 + hour + min / 60 + sec / 3600.0;
}

long long getTotalIp (const std::string& ip) {
    // 192.0.2.10:
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

    ip1 = stoi(strIp1);
    ip2 = stoi(strIp2);
    ip3 = stoi(strIp3);
    ip4 = stoi(strIp4);

    return ip1*pow(255, 3) + ip2*pow(255, 2) + ip3*255 + ip4;
}

/** 
* @brief Reads the input file, stores the logs in a vector, sorts them, 
* saved them inside an output file and searches for a user-provided date range.
*
* @param file_name Name of the input file containing the logs. 
* @return logs vector with Log objects from the bitacora.txt file
*/
std::vector<Log*> getVector(const std::string& file_name) {
    std::vector<Log*> logs;
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cout << "Couldn't open the file \n";
        exit(1);
    }

    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string month = "", strDay = "", strHour = "", strMin = "", strSec = "", ip = "", host = "", msg = "";
        double day = 0, hour = 0, min = 0, sec = 0;

        getline(ss, month, ' ');
        getline(ss, strDay, ' ');
        getline(ss, strHour, ':');
        getline(ss, strMin, ':');
        getline(ss, strSec, ' ');
        getline(ss, ip, ' ');
        getline(ss, host, ' ');
        getline(ss, msg);

        day = stoi(strDay);
        hour = stoi(strHour);
        min = stoi(strMin);
        sec = stoi(strSec);
        double integerMonth = monthToInt(month);
        double totTime = getTotalTime(integerMonth, day, hour, min, sec);
        long long totIp = getTotalIp(ip);
        Log* log = new Log(month, integerMonth, day, hour, min, sec, totTime, ip, totIp, host, msg);
        logs.push_back(log);
    }
    file.close();
    return logs;
}

template <typename T>
void printUserRange(const std::vector<T>& logsVect, Criteria printCriteria) {
    switch (printCriteria) {
        case Criteria::DATE : {
            std::string startMonth = "", endMonth = "";
            int startDay = 0, endDay = 0;

            std::cout << "Enter start month (e.g. Sep) and day (e.g. 10): ";
            std::cin >> startMonth >> startDay;

            std::cout << "Enter end month (e.g. Sep) and day (e.g. 10): ";
            std::cin >> endMonth >> endDay;

            double startTime = getTotalTime(monthToInt(startMonth), startDay, 0, 0, 0);
            double endTime = getTotalTime(monthToInt(endMonth), endDay, 23, 59, 59);

            auto [startIdx, endIdx] = binarySearch(logsVect, startTime, endTime, Criteria::DATE);
            if (startIdx != -1) {
                std::vector<Log*> rangeDateVector = {};
                for (int i = startIdx; i <= endIdx; i++) {
                    rangeDateVector.push_back(logsVect[i]);
                    std::cout << *logsVect[i];
                }
                saveSortedLogs(rangeDateVector, "orderedLogsbyRangedDate.txt");
            }
            else {
                std::cout << "No logs found for the given range. \n";
            }

            // Releases memory from the heap
            for (Log* log : logsVect) {
                delete log;
            }
            break;
        }
        case Criteria::IP : {
            std::string startIp = "", endIp = "";
            
            std::cout << "Enter start IP (Ej. 192.0.2.10:0000): ";
            std::cin >> startIp;

            std::cout << "Enter end IP (Ej. 192.0.2.11:0000): ";
            std::cin >> endIp;

            long long totalStartIp = getTotalIp(startIp);
            long long totalEndIp = getTotalIp(endIp);

            auto [startIdx2, endIdx2] = binarySearch(logsVect, totalStartIp, totalEndIp, Criteria::IP);
            if (startIdx2 != -1) {
                std::vector<Log*> rangeIpVector = {};
                for (int i = startIdx2; i <= endIdx2; i++) {
                    rangeIpVector.push_back(logsVect[i]);
                    std::cout << *logsVect[i];
                }
                saveSortedLogs(rangeIpVector, "orderedLogsbyRangedIp.txt");
            }
            else {
                std::cout << "No logs found for the given range. \n";
            }
            // Releases memory from the heap
            for (Log* log : logsVect) {
                delete log;
            }

            break;
        }

    } 
}

/**
 * @brief Initializes the code by sending the filename to getVector().
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments. argv[0] contains the path to the executable file and argv[1] contains the path to the input file.
 * @return 0 to finalize.
 */
int main(int argc, char* argv[]) {
        if (argc < 2) {
        std::cout << "Usage: ./main <input_file>\n";
        return 1;
    }
    int option = 0;
    do {
        std::cout << "\nSelect an option: " << "\n"
        << "1. Search by date" << "\n"
        << "2. Search by IP" << "\n"
        << "3. Exit" << "\n";
        std::cin >> option;

        if (option == 1) {
            std::vector<Log*> logsVector = getVector(argv[1]);
            // Order the logsVector
            mergeSort(logsVector , 0, static_cast<int>(logsVector.size()) - 1, Criteria::DATE);
            // Save the sorted logs into an output file named orderedLogs.txt
            saveSortedLogs(logsVector, "orderedLogsByData.txt");
            // Prompts for a date range and prints if it's found
            printUserRange(logsVector, Criteria::DATE);
        } else if (option == 2) {
            std::vector<Log*> logsVector = getVector(argv[1]);
            mergeSort(logsVector, 0, static_cast<int>(logsVector.size()) - 1, Criteria::IP);
            saveSortedLogs(logsVector, "orderedLogsByIP.txt");
            printUserRange(logsVector, Criteria::IP);
        } else if(option == 3) {
            std::cout << "I'll be back";
        } else {
            std::cout << "Invalid option\n";
        }

    } while(option != 3);

    return 0;
}
