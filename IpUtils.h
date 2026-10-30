#pragma once
#include <sstream>
#include <string>
#include <cmath>

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