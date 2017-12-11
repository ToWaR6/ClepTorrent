#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "functionFile.h"

int main(int argc, char const *argv[]) {

	if (argc < 2) {//Si le processus a moins de 2 paramètres, on affiche les usages et on coupe le processus
		printf("%s -port\n", argv[0]);
		return -1;
	}
	printf("Lancement du serveur\n");
	int dS = socket(AF_INET, SOCK_STREAM, 0);//Création de la socket
	if (dS < 0) {
		perror("socket()");
		return -1;
	}
	printf("Socket créée\n");
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = (short) htons(atoi(argv[1]));//Port

	int err = bind(dS, (struct sockaddr*)&ad, sizeof(ad));//On bind la socket au port passé en paramètre
	if (err < 0) {
		perror("bind()");
		close(dS);
		return -1;
	}
	printf("La socket est maintenant nommé\n");
	int nbClient = 1;
	err = listen(dS, nbClient);//Permet de savoir combien de client peut télécharger un fichier
	if (err < 0) {
		perror("listen()");
		close(dS);
		return -1;
	}
	printf("Le serveur est prêt à recevoir %d clients\n", nbClient);

	struct sockaddr_in adClient;
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;//Lorsque le client se connecte adClient sera initialisé
	if(dSClient<0){
		perror("accept ");
		return -1;
	}

	printf("Un client s'est connecté\n");
	

	int res = myReceivFile(dSClient);//cf FunctionFile.h
	if (res == -1) {
		perror("ERREUR myReceivFile");
		close(dS);
		close(dSClient);
		return -1;
	}

	printf("\n");

	//Fermeture des sockets
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
