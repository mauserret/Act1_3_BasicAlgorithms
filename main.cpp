/* This program reads the data set called "bitacora.txt" that contaings different logs, saves it in a vector with pointers to class Log, then
it orders the logs from the earliest to the latest date and prompts the user for a date range (e.g. Sep 10 to Sep 15), the the program will determine
if there's any logs for the given range, prints them into console and creates an output file named "orderedRangedLogs.txt" containing the logs inside
the provided range.

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

/**
 * @class Log
 * @brief Represents a single server log entry containing timestamps, ip address, host, and message.
 */
class Log {
private:
    std::string month;
    double doubleMonth;
    double day;
    double hour;
    double min;
    double sec;
    double totalTime;
    std::string ip;
    std::string host;
    std::string message;

public:
    Log(std::string month, double doubleMonth, double day, double hour, double min, double sec, double totalTime,
        std::string ip, std::string host, std::string message) :
        month{ month }, doubleMonth{ doubleMonth }, day{ day }, hour{ hour }, min{ min }, sec{ sec }, totalTime{ totalTime },
        ip{ ip }, host{ host }, message{ message } {
    }

    /**
     * @brief Overloads the stream insertion operator to format the output for the log entry.
     * @param os Output stream reference where the output will be printed.
     * @param log The log to format it's output.
     * @return std::ostream& reference to the modified output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Log& log) {
        os << log.getMonth() << " "
            << log.getDay() << " "
            << log.getHour() << ":"
            << log.getMin() << ":"
            << log.getSec() << " "
            << log.getIp() << " "
            << log.getHost() << " "
            << log.getMessage() << " "
            << "\n";
        return os;
    }

    std::string getMonth() const { return month; }
    double getdoubleMonth() const { return doubleMonth; }
    double getDay() const { return day; }
    double getHour() const { return hour; }
    double getMin() const { return min; }
    double getSec() const { return sec; }
    double getTotalTime() const { return totalTime; }
    std::string getIp() const { return ip; }
    std::string getHost() const { return host; }
    std::string getMessage() const { return message; }
};

/**
 * @brief Sorts a vector with the merge sort algorithm by using recursion to divide and conquer, uses merge() function.
 *
 * @param v Reference to the vector to sort.
 * @param l Starting index of the range to sort.
 * @param r Ending index of the range to sort.
 */
template <typename T>
void mergeSort(std::vector<T>& v, int l, int r) {
    if (l >= r) return;
    int mid = l + (r - l) / 2;
    mergeSort(v, l, mid);
    mergeSort(v, mid + 1, r);
    merge(v, l, mid, r);
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
void merge(std::vector<T>& v, int l, int mid, int r) {
    std::vector<T> temp(r - l + 1);
    int i = l;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= r) {
        if (v[i]->getTotalTime() <= v[j]->getTotalTime()) {
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
 * @brief Searches for the first and last log inside a given time range using binary search. 
 * 
 * @param v Reference to the vector containing the sorted logs. 
 * @param start Beginning of the desired time range. 
 * @param end End of the desired time range. 
 * @return std::pair<int, int> Pair containing the first and last indexes found. 
 */
template <typename T>
std::pair<int, int> binarySearch(const std::vector<T>& v, double start, double end) {
    int startIndex = -1;
    int endIndex = -1;

    int l = 0;
    int r = v.size() - 1;

    while (l <= r) {
        int m = l + (r - l) / 2;
        if (v[m]->getTotalTime() >= start) {
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
        int m = l + (r - l) / 2;
        if (v[m]->getTotalTime() <= end) {
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

/** 
* @brief Reads the input file, stores the logs in a vector, sorts them, 
* searches for a user-provided date range and creates an output file. 
*
* @param file_name Name of the input file containing the logs. 
*/
void getVector(const std::string& file_name) {
    std::vector<Log*> logs;
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cout << "Couldn't open the file \n";
        return;
    }

    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string month, strDay, strHour, strMin, strSec, ip, host, msg;
        double day, hour, min, sec;

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

        Log* log = new Log(month, integerMonth, day, hour, min, sec, totTime, ip, host, msg);
        logs.push_back(log);
    }
    file.close();

    mergeSort(logs, 0, logs.size() - 1);

    std::string startMonth, endMonth;
    int startDay, endDay;

    std::cout << "Enter start month (e.g. Sep) and day (e.g. 10): ";
    std::cin >> startMonth >> startDay;

    std::cout << "Enter end month (e.g. Sep) and day (e.g. 10): ";
    std::cin >> endMonth >> endDay;

    double startTime = getTotalTime(monthToInt(startMonth), startDay, 0, 0, 0);
    double endTime = getTotalTime(monthToInt(endMonth), endDay, 23, 59, 59);

    auto [startIdx, endIdx] = binarySearch(logs, startTime, endTime);
    if (startIdx != -1) {
        std::ofstream orderedFile("orderedRangedLogs.txt");
        if (orderedFile.is_open()) {
            for (int i = startIdx; i <= endIdx; i++) {
                std::cout << *logs[i];

                orderedFile << *logs[i];
            }
        }
        else {
            std::cout << "Couldn't create a file for the ordered logs \n";
        }

    }
    else {
        std::cout << "No logs found for the given range. \n";
    }
    // Releases memory from heap
    for (Log* log : logs) {
        delete log;
    }
}

/**
 * @brief Initializes the code by sending the filename to getVector()
 */
int main() {
    getVector("bitacora.txt");
    return 0;
}
