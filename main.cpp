#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

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
            month{month}, doubleMonth{doubleMonth}, day{day}, hour{hour}, min{min}, sec{sec}, totalTime{totalTime},
            ip{ip}, host{host}, message{message} {}

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

template <typename T>
void mergeSort(std::vector<T>& v, int l, int r) {
    if (l >= r) return;
    int mid = (l + r) / 2;
    mergeSort(v, l, mid);
    mergeSort(v, mid + 1, r);
    merge(v, l, mid, r);
}

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

template <typename T>
std::pair<int,int> binarySearch(const std::vector<T>& v, double start, double end) {
    int startIndex = -1;
    int endIndex = -1;

    int l = 0;
    int r = v.size()-1;

    while (l <= r) {
        int m = l + (r - l) / 2;
        if (v[m]->getTotalTime() >= start) {
            startIndex = m;
            r = m - 1;
        } else {
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
        } else {
            r = m - 1;
        }
    }

    if (startIndex == -1 || endIndex == - 1 || startIndex > endIndex) {
        return {-1, -1};
    }
    return {startIndex, endIndex};
} 

int monthToInt(std::string& m) {
    std::unordered_map<std::string, int> convert {
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
        {"Dic", 334},
    };
    return convert[m];
}

double getTotalTime(const double& month, const double& day, const double& hour, const double& min, const double& sec) {
    double dMin = min/60;
    double dSec = sec/3600;
    return (month + day) * 24 + hour + dMin + dSec;
}

void getVector(const std::string& file_name) {
    std::vector<Log*> logs;
    std::ifstream file(file_name);
    if(!file.is_open()) {
        std::cout << "Couldn't open the file \n";
    }

    std::string line;
    while(getline(file, line)) {
        std::stringstream ss(line);
        //std::cout << line << "\n";
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
        std::ofstream orderedFile("orderedLogs.txt");
        if (orderedFile.is_open()) {
            for (int i = startIdx; i <= endIdx; i++) {
                std::cout << *logs[i];

                orderedFile << *logs[i];
            }
        } else {
            std::cout << "Couldn't create a file for the ordered logs \n";
        }

    } else {
        std::cout << "No logs found for the given range. \n";
    }

    for (Log* log: logs) {
        delete log;
    }     
}

int main() {
    getVector("bitacora.txt");
    return 0;
}

