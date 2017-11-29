#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "functionFile.h"

int main(int argc, char const *argv[]) {

	if (argc < 2) {
		printf("%s -port\n", argv[0]);
		return -1;
	}

	int dS = socket(PF_INET, SOCK_STREAM, 0);
	if (dS < 0) {
		perror("socket()");
		return -1;
	}

	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons((short) atoi(argv[1]));

	int err = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
	if (err < 0) {
		perror("bind()");
		close(dS);
		return -1;
	}

	err = listen(dS, 1);
	if (err < 0) {
		perror("listen()");
		close(dS);
		return -1;
	}
	printf("Server ready\n");
	struct sockaddr_in adClient;
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;

	printf("Client connected\n");
	

	int res = myReceiv(dSClient);
	if (res == -1) {
		perror("ERREUR myReceiv");
		close(dS);
		close(dSClient);
		return -1;
	}

	printf("\n");

	if(close(dS)<0){
		perror("close ds");
		return -1;
	}
	if(close(dSClient)<0){
		perror("close dsClient");
		return -1;
	}
	printf("Server close\n");
	return 0;
}
