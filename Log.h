#pragma once
#include <string>
#include <iostream> 
#include <iomanip>
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