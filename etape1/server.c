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
	printf("Lancement du serveur\n");
	int dS = socket(AF_INET, SOCK_STREAM, 0);
	if (dS < 0) {
		perror("socket()");
		return -1;
	}
	printf("Socket créée\n");
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = (short) htons(atoi(argv[1]));

	int err = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
	if (err < 0) {
		perror("bind()");
		close(dS);
		return -1;
	}
	printf("La socket est maintenant nommé\n");
	int nbClient = 1;
	err = listen(dS, nbClient);
	if (err < 0) {
		perror("listen()");
		close(dS);
		return -1;
	}
	printf("Le serveur est prêt à recevoir %d clients\n", nbClient);

	struct sockaddr_in adClient;
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;
	if(dSClient<0){
		perror("accept ");
		return -1;
	}

	printf("Un client s'est connecté\n");
	

	int res = myReceivFile(dSClient);
	if (res == -1) {
		perror("ERREUR myReceivFile");
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
	printf("Fermeture du serveur\n");
	return 0;
}
