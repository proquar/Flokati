#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define SLIP_END (char)0xC0
#define SLIP_ESC (char)0xDB

int running=1;
int mcast_port, serial_port;

struct sockaddr_in mcast_addr;

void exit_program(int signal) {
	printf("Exiting...");
	running=0;
	close(mcast_port);
	close(serial_port);
}

void set_mcast_addr() {
	memset(&mcast_addr,0,sizeof(mcast_addr));
	mcast_addr.sin_family=AF_INET;
	mcast_addr.sin_addr.s_addr=inet_addr("233.133.133.133");
	mcast_addr.sin_port=htons(5331);
}

int open_mcast() {
	struct ip_mreq mreq;
	int true=1; int false=0;
	int ttl=2;
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
// 	addr.sin_port=mcast_addr.sin_port;
	addr.sin_port=htons(5331);
	
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
	
// 	mreq.imr_multiaddr.s_addr=mcast_addr.sin_addr.s_addr;
	mreq.imr_multiaddr.s_addr=inet_addr("233.133.133.133");
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if (setsockopt(mcast_port, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("Multicast: Error joining group.");
		close(mcast_port);
		return 0;
	}
	
	fcntl(mcast_port, F_SETFL, O_NONBLOCK);
	
	return 1;
}

int open_serial(char *host, int port) {
	// connect to a running ser2net
	struct hostent *server;
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	
	if ((serial_port = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Serial:    Error opening socket.");
		return 0;
	}
	
	if ((server = gethostbyname(host))==NULL) {
		perror("Serial:    Error getting IP from host.");
		close(serial_port);
		return 0;
	}
	memcpy(&(addr.sin_addr.s_addr), server->h_addr, server->h_length);
	
	if (connect(serial_port, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Serial:    Error connecting to host.");
		close(serial_port);
		return 0;
	}
	fcntl(serial_port, F_SETFL, O_NONBLOCK);
	
	return 1;
}

void setBrightness(const char *file, int ness) {
	FILE *led;
	
	led=fopen(file,"w");
	if (led) {
		fprintf(led, "%i",ness);
		fflush(led);
		fclose(led);
	} else {
		if (ness) printf("Cannot open brightness-file.\n");
	}
}

int main(int argc, char *argv[]) {
	
	fd_set connections;
	int max_sock, len, bufpos, i;
	char fromradio[33], toradio[33];
	char buffer[130];
	int escaped=0;
	int fromradio_pos=0;
	int serial_open=0;
	
	struct timespec timeout;
	timeout.tv_sec=3;
	timeout.tv_nsec=0;
	int clock;
	
	if (argc<3) {
		printf("Need at least 2 arguments (%i given).\n", argc-1);
		printf("Usage: flokati-bridge host port [led-file]\n");
		printf("This is Flokati version 2.0.0.\n");
		return 1;
	}
	
	(void) signal(SIGINT, exit_program);
	
	set_mcast_addr();
	if (!open_mcast()) {
		printf("Could not open multicast socket.\n");
		return 1;
	}
	if (open_serial(argv[1], atoi(argv[2]))) {
		printf("Connected to ser2net.\n");
		if (argc==4) setBrightness(argv[3], 255);
		serial_open=1;
	} else {
		printf("Could not connect to ser2net (yet).\n");
		serial_open=0;
	}
	if (serial_open==0 || mcast_port>=serial_port) max_sock=mcast_port+1;
	else max_sock=serial_port+1;
	
	clock=(int)time(NULL);
	
	while(running) {
		FD_ZERO(&connections);
		FD_SET(mcast_port,&connections);
		if (serial_open) FD_SET(serial_port,&connections);
		
		if (pselect(max_sock, &connections, NULL, NULL, &timeout, NULL) > 0 ) {
			if (FD_ISSET(mcast_port, &connections)) {
				if ((len=read(mcast_port, toradio, 33)) > 0) {
					bufpos=0;
					for (i=0; i<len; i++) {
						if (toradio[i]==SLIP_END || toradio[i]==SLIP_ESC) {
							buffer[bufpos]=SLIP_ESC;
							bufpos++;
						}
						buffer[bufpos]=toradio[i];
						bufpos++;
					}
					buffer[bufpos]=SLIP_END;
					if (serial_open) write(serial_port, buffer, bufpos+1);
					
// 					for (i=0; i<bufpos+1; i++)
// 						printf("%02x ",buffer[i]&0xff);
// 					
// 					printf(" <-\n");
				}
			}
			if (serial_open && FD_ISSET(serial_port, &connections)) {
				if ((len=read(serial_port, buffer, 130)) > 0) {
					clock=(int)time(NULL);
					for (i=0; i<len; i++) {
						if (buffer[i]==SLIP_ESC) {
							escaped=1;
							continue;
						}
						
						if (buffer[i]==SLIP_END && escaped==0) {
							sendto(mcast_port, fromradio, fromradio_pos+1, 0,
								(struct sockaddr *)&mcast_addr, sizeof(mcast_addr));
// 							printf("   -> (%i)\n",fromradio_pos+1);
							fromradio_pos=0;
							continue;
						}
						
// 						printf("%02x ",buffer[i]&0xff);
						fromradio[fromradio_pos]=buffer[i];
						if (fromradio_pos<32) fromradio_pos++;
						escaped=0;
					}
				}
			}
		}
		
		if (((int)time(NULL)-clock)>5) {
			printf("Timeout. Reconnecting to ser2net.\n");
			
			if (argc==4) setBrightness(argv[3], 0);
			
			close(serial_port);
			serial_open=0;
			if (open_serial(argv[1], atoi(argv[2]))) {
				printf("Connected.\n");
				if (argc==4) setBrightness(argv[3], 255);
				serial_open=1;
			} else {
				printf("Could not reconnect.\n");
				serial_open=0;
			}
				
			if (serial_open==0 || mcast_port>=serial_port) max_sock=mcast_port+1;
			else max_sock=serial_port+1;
			
			clock=(int)time(NULL);
		}
	}
	
	if (argc==4) setBrightness(argv[3], 0);
	
	printf(" Stopped.\n");
	return 0;
}
