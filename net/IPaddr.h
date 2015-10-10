//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_IPADDR_H
#define GALILEO_TERREMOTI_IPADDR_H

#include <stdint.h>
#include <stdio.h>
#include <string>

/**
 * IP address type
 */
class IPaddr {
public:

	/**
	 * Create a new IP address with empty value
	 */
	IPaddr();

	/**
	 * Create a new IP address with octects specified
	 * @param a1 First octect
	 * @param a2 Second octect
	 * @param a3 Third octect
	 * @param a4 Forth octect
	 */
	IPaddr(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
	//IPaddr(int, int, int, int);

	/**
	 * Create a new IP address from integer
	 * @param x Unsigned 32-bit integer representing IP address
	 */
	IPaddr(uint32_t x);

	/**
	 * Set IP address from int
	 * @param u Unsigned 32-bit integer ip address
	 */
	void setInt(uint32_t u) { ipaddr = u; };

	/**
	 * Get IP address as integer
	 * @return IP address as unsigned 32-bit integer
	 */
	uint32_t asInt() const { return ipaddr; };

	/**
	 * Get IP address representation as 4-octect group string (i.e. 192.0.2.1)
	 * @return IP address as string
	 */
	std::string asString();

	/**
	 * Get IP address representation as 4-octect group string (i.e. 192.0.2.1)
	 * @return IP address as string
	 */
	operator std::string() { return asString(); };

	/**
	 * Get IP address as integer
	 * @return IP address as unsigned 32-bit integer
	 */
	operator uint32_t() const;

	/**
	 * Set IP address from int
	 * @param address Unsigned 32-bit integer ip address
	 */
	IPaddr& operator=(uint32_t address);

	/**
	 * Access to single octects of IP address
	 * @param idx Octect index (0-3)
	 */
	uint8_t operator[](int idx) const { return ((uint8_t*)&ipaddr)[idx]; };

	/**
	 * Compare IP address to this object
	 * @param addr IP address to compare to us
	 */
	bool operator==(IPaddr& addr) const { return addr.asInt() == ipaddr; };

	/**
	 * Resolve Hostname to IP address
	 * @param hostname Hostname to resolve
	 * @return IP address class populated with the first DNS A record (eg. IPv4 address)
	 */
	static IPaddr resolve(std::string hostname);

	/**
	 * Returns the local IP address
	 * @return Local IP Address
	 */
	static IPaddr localIP();
private:
	uint32_t ipaddr;
};

#endif //GALILEO_TERREMOTI_IPADDR_H

