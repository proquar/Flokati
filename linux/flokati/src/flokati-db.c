#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>

#include <json/json.h>
#include "flokati-db-seedfile.h"

int mcast_port, running;

struct sockaddr_in mcast_addr;

void exit_program(int signal) {
	printf("Exiting...");
	running=0;
	close(mcast_port);
}

void set_mcast_addr() {
	memset(&mcast_addr,0,sizeof(mcast_addr));
	mcast_addr.sin_family=AF_INET;
	mcast_addr.sin_addr.s_addr=inet_addr("233.133.133.133");
	mcast_addr.sin_port=htons(5332);
}

int open_mcast() {
	struct ip_mreq mreq;
	int true=1; int false=0;
	int ttl=2;
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(5332);
	
	if ((mcast_port = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Multicast: Error opening socket.");
		return 0;
	}
	
	if (setsockopt(mcast_port, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)) < 0) {
		perror("Multicast: Error setting REUSEADDR.");
		close(mcast_port);
		return 0;
	}
	if (setsockopt(mcast_port, IPPROTO_IP, IP_MULTICAST_LOOP, &false, sizeof(false)) < 0) {
		perror("Multicast: Error disabling loopback.");
		close(mcast_port);
		return 0;
	}
	if (setsockopt(mcast_port, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("Multicast: Cannot raise tt (all this is pointless without tt>1).");
		close(mcast_port);
		return 0;
	}
	
	if (bind(mcast_port, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Multicast: Error binding.");
		close(mcast_port);
		return 0;
	}
	
	mreq.imr_multiaddr.s_addr=inet_addr("233.133.133.133");
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if (setsockopt(mcast_port, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("Multicast: Error joining group.");
		close(mcast_port);
		return 0;
	}
	
	return 1;
}

int main(int argc, char *argv[]) {
	char buffer[500];
	struct json_object *request, *rq_doc;
	char *reqid_s, *reqid_s2;
	uint64_t reqid;
	char *result;
	long result_len;
	
	(void) signal(SIGINT, exit_program);
	
	if (argc>1)
		db_init(argv[1]);
	else {
		printf("No directory for device-data specified.\n");
		return 1;
	}
	
	set_mcast_addr();
	if (!open_mcast()) {
		printf("Could not open multicast socket.\n");
		return 1;
	}
	running=1;
	
	while(running) {
		if (read(mcast_port, buffer, 500) > 0) {
			rq_doc = json_tokener_parse(buffer);
			request = json_object_object_get(rq_doc, "request");
			reqid_s = (char *)json_object_get_string(request);
			
			if(reqid_s)
				reqid = (uint64_t)strtoull(reqid_s, &reqid_s2, 16);
			
			json_object_put(rq_doc);
			
			if (reqid_s && (reqid_s < reqid_s2) && ((reqid_s2-16) <= reqid_s)) {
				// check that request ID is between 1 and 16 chars long
				
				result_len = db_get(reqid, &result);
				
				if (result_len>0) {
					sendto(mcast_port, result, result_len, 0,
						(struct sockaddr *)&mcast_addr, sizeof(mcast_addr));
					printf("Replied to request %08x%08x\n",(unsigned int)(reqid>>32),(unsigned int)reqid);
				}
			}
		}
	}
	
	printf("\n");
	
	return 0;
}