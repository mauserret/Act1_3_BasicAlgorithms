/**
 * Program: Helper functions that validate the octets of an IPv4 address and
 *          convert the address into a single number usable as a tree key.
 * Authors: A01648241
 *          A01642638
 *          A01648221
 *          A01643073
 * Date: 31/10/2026
*/

#pragma once
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>

const int MAX_OCTET = 255;
const long long OCTET_RANGE = 256;

/**
 * @brief Checks that a text is a whole number between 0 and 255.
 * @param octet Text of one IP octet.
 * @return Numeric value of the octet.
 * @throws std::invalid_argument If the text is not a number, has extra
 *         characters or is outside 0-255.
 * @note Time O(1) because an octet has a bounded size. Space O(1).
 */
inline int checkOctet(const std::string& octet) {
	try {
		std::size_t octetSize = 0;
		int ipOctet = std::stoi(octet, &octetSize);

		if (octetSize != octet.size() || ipOctet < 0 || ipOctet > MAX_OCTET) {
			throw std::invalid_argument("Invalid octet IP: " + octet);
		}
		return ipOctet;
	} catch (const std::exception&) {
		throw std::invalid_argument("Invalid octet IP: " + octet);
	}
}

/**
 * @brief Converts an IPv4 address, with or without a port, into a number.
 *        Octets are combined as a base 256 number, so numeric order matches
 *        the order of the addresses.
 * @param ip Text such as "192.0.2.10" or "192.0.2.10:8080".
 * @return Numeric value of the address, the port is ignored.
 * @throws std::invalid_argument If any octet is invalid.
 * @note Time O(1) because an address always has four octets. Space O(1).
 */
inline long long getTotalIp(const std::string& ip) {
	std::string firstOctet = "";
	std::string secondOctet = "";
	std::string thirdOctet = "";
	std::string fourthOctet = "";
	std::stringstream ss(ip);

	std::getline(ss, firstOctet, '.');
	std::getline(ss, secondOctet, '.');
	std::getline(ss, thirdOctet, '.');
	std::getline(ss, fourthOctet, ':');

	long long total = checkOctet(firstOctet);
	total = total * OCTET_RANGE + checkOctet(secondOctet);
	total = total * OCTET_RANGE + checkOctet(thirdOctet);
	total = total * OCTET_RANGE + checkOctet(fourthOctet);
	return total;
}