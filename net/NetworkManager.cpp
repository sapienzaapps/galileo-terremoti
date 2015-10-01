#include <vendor_specific.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <resolv.h>
#include <netinet/ip_icmp.h>
#include "NetworkManager.h"
#include "../Log.h"
#include "../Utils.h"

#ifdef __IS_GALILEO
#include "../Utils.h"
#endif

struct ICMP_PACKET {
	struct icmphdr hdr;
	char msg[64-sizeof(struct icmphdr)];
};

float NetworkManager::lastLatency = -1;
bool NetworkManager::connectionAvailable = false;
bool NetworkManager::connectionChecked = false;

bool NetworkManager::isConnectedToInternet() {
	return NetworkManager::isConnectedToInternet(false);
}

bool NetworkManager::isConnectedToInternet(bool force) {
	if (!NetworkManager::connectionChecked || force) {
		NetworkManager::connectionAvailable = false;

		int ping = system(CMD_PING);

		Log::d("Ping WIFEXITED WEXITSTATUS: %i %i", WIFEXITED(ping), WEXITSTATUS(ping));
		if (WEXITSTATUS(ping) == 0) {
			NetworkManager::connectionAvailable = true;
		}
		NetworkManager::connectionChecked = true;
	}
	return NetworkManager::connectionAvailable;
}

void NetworkManager::init() {
	signal(SIGPIPE, SIG_IGN);
}

float NetworkManager::getLastLatency() {
	return lastLatency;
}

float NetworkManager::latency() {
	unsigned long startms = Utils::millis();
	ping(IPaddr(8, 8, 8, 8), 2000, 1);
	lastLatency = Utils::millis() - startms;
	return lastLatency;
}

int NetworkManager::ping(IPaddr address, unsigned int waitms, uint16_t sequenceNumber) {
	const int val = 255;
	struct ICMP_PACKET pckt;
	struct sockaddr_in r_addr;
	struct sockaddr_in addr_ping;

	pid_t pid = getpid();
	struct protoent* proto = getprotobyname("ICMP");

	bzero(&addr_ping, sizeof(addr_ping));

	addr_ping.sin_family = AF_INET;
	addr_ping.sin_port = 0;
	addr_ping.sin_addr.s_addr = htonl(address);

	int sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 ) {
		perror("socket");
		return 1;
	}
	if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0) {
		perror("Set TTL option");
		return 1;
	}
	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 ) {
		perror("Request nonblocking I/O");
		return 1;
	}

	bzero(&pckt, sizeof(pckt));
	pckt.hdr.type = ICMP_ECHO;
	pckt.hdr.un.echo.id = (uint16_t)pid;

	unsigned int i;
	for (i = 0; i < sizeof(pckt.msg)-1; i++ ) {
		pckt.msg[i] = i+'0';
	}

	pckt.msg[i] = 0;
	pckt.hdr.un.echo.sequence = sequenceNumber;
	pckt.hdr.checksum = NetworkManager::checksum(&pckt, sizeof(pckt));
	if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr_ping, sizeof(addr_ping)) <= 0 ) {
		perror("sendto");
	}

	unsigned long startms = Utils::millis();
	while(Utils::millis() < startms + waitms) {
		socklen_t len = 0;
		if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 ) {
			// FIXME: check ICMP type?
			return 0;
		}
		usleep(100);
	}

	return 1;
}

unsigned short NetworkManager::checksum(void *b, int len) {
	unsigned short *buf = (unsigned short*)b;
	unsigned int sum=0;

	for ( sum = 0; len > 1; len -= 2 ) {
		sum += *buf++;
	}
	if ( len == 1 ) {
		sum += *(unsigned char*)buf;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return ~sum;
}