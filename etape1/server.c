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

	int dS = socket(AF_INET, SOCK_STREAM, 0);
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

	err = listen(dS, 2);
	if (err < 0) {
		perror("listen()");
		close(dS);
		return -1;
	}
	printf("Server ready\n");
	struct sockaddr_in adClient;
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;
	printf("Client connecté\n");
	char *filename = "rsc/16MO(copie).txt";
	FILE* fp = fopen(filename, "w");
	if(fp==NULL){
		perror("Ouverture fichier");
		close(dS);
		exit(-1);
	}


	int res = myReceiv(dSClient, fp);
	if (res == -1) {
		perror("ERREUR myReceiv");
		close(dS);
		close(dSClient);
		return -1;
	}

	printf("\n");

	fclose(fp);
	close(dS);
	close(dSClient);
	return 0;
}
