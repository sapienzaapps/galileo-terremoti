//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_IPADDR_H
#define GALILEO_TERREMOTI_IPADDR_H

#include <stdint.h>
#include <stdio.h>
#include <string>

class IPaddr {
public:
	IPaddr();
	IPaddr(uint8_t, uint8_t, uint8_t, uint8_t);
	//IPaddr(int, int, int, int);
	IPaddr(uint32_t x);

	void setInt(uint32_t u) { ipaddr = u; };
	uint32_t asInt() const { return ipaddr; };
	std::string asString();

	operator std::string() { return asString(); };
	operator uint32_t() const;
	IPaddr& operator=(uint32_t address);
	uint8_t operator[](int idx) const { return ((uint8_t*)&ipaddr)[idx]; };

	bool operator==(IPaddr& addr) { return addr.asInt() == ipaddr; };

	static IPaddr resolve(std::string hostname);
	static IPaddr localIP();
private:
	uint32_t ipaddr;
};

#endif //GALILEO_TERREMOTI_IPADDR_H

