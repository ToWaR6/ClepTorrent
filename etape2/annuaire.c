#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <string.h> // strlen()
#include <sys/types.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <sys/socket.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()
#include "functionFile.h"

struct pairData {
	struct sockaddr_in pair;
	int nbFile;
	char** fileList;
};
int newPair(const struct pairData *pData,int nbPair,const struct sockaddr_in *addrCli){
	int ipClient = addrCli->sin_addr.s_addr;
	int ipPair;

	unsigned short portClient = addrCli->sin_port;
	unsigned short portPair;

	for (int i = 0; i < nbPair; i++){
		ipPair = pData[i].pair.sin_addr.s_addr;
		portPair = pData[i].pair.sin_port;
		// printf("Ip : %d = %d\n",ipPair,ipClient);
		// printf("Port : %d = %d \n",portPair,portClient );
		if(portPair==portClient  &&  ipPair==ipClient ){
			return 0;
		}
	}
	return 1;
}
int main(int argc, char const *argv[]) {
	if(argc < 3) {
		printf("Usage: %s <PORT> <nbPairMax>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// variable du serveur
	int nbMaxPair = atoi(argv[2]);
	struct pairData pData[nbMaxPair];
	int nbPair = 0;
	int lastFreeId = 0;

	printf("Creation de la socket.....");
	int sockServ = socket(AF_INET, SOCK_STREAM, 0);
	if(sockServ == -1) {
		printf("fail\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	struct sockaddr_in addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(atoi(argv[1]));
	addrServ.sin_addr.s_addr = INADDR_ANY;

	printf("Bind de la socket.....");
	int testBind = bind(sockServ, (struct sockaddr*)&addrServ, sizeof(addrServ));
	if(testBind == -1) {
		printf("fail\n");
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	printf("Listen de la socket.....");
	int testListen = listen(sockServ, 3);
	if(testListen == -1) {
		printf("fail\n");
		perror("listen()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	while(1) {
		printf("En attente d'un client\n");
		struct sockaddr_in addrCli;
		socklen_t lenAddrCli = sizeof(struct sockaddr_in);
		int sockCli = accept(sockServ, (struct sockaddr*)&addrCli, &lenAddrCli);
		if(sockCli == -1) {
			perror("accept()");
			exit(EXIT_FAILURE);
		}

		struct sockaddr_in addrCli2;
		memcpy(&addrCli2,&addrCli,lenAddrCli);

		// ****************
		// * RECV FICHIER *
		// ****************

		// recv du port
		short port = 0;
		int testRecv = recv(sockCli, &port, sizeof(short), 0);
		if(testRecv == -1) {
			perror("recv()");
			exit(EXIT_FAILURE);
		}
		char str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET,&addrCli.sin_addr,str,INET6_ADDRSTRLEN);
		printf("Le client sera contacté avec %s:%d \n",str,port);
		addrCli2.sin_port = port;
		// recv du nombre de fichier
		if(nbPair<nbMaxPair && newPair(pData,nbPair,&addrCli2)==1){
			nbPair++;
			memcpy(&pData[lastFreeId].pair,&addrCli2,lenAddrCli);
			
			printf("addrCli ip : %d = %d\n",addrCli.sin_addr.s_addr,pData[0].pair.sin_addr.s_addr);
			testRecv = recv(sockCli, &pData[lastFreeId].nbFile, sizeof(int), 0);
			if(testRecv == -1) {
				perror("recv()");
				exit(EXIT_FAILURE);
			}
			printf("Le client a %d fichiers\n", pData[lastFreeId].nbFile);

			pData[lastFreeId].fileList = (char**)malloc(pData[lastFreeId].nbFile * sizeof(char*));

			// for chaque fichier
			for(int i = 0; i < pData[lastFreeId].nbFile; i++) {
				// recv la taille du nom
				int nameSize = 0;
				testRecv = recv(sockCli, &nameSize, sizeof(int), 0);
				if(testRecv == -1) {
					perror("recv()");
					exit(EXIT_FAILURE);
				}
				// printf("done\n");
				

				pData[lastFreeId].fileList[i] = (char*)malloc(nameSize * sizeof(char));

				// recv nom
				testRecv = myLoopReceiv(sockCli, pData[lastFreeId].fileList[i], nameSize, 0);
				if(testRecv == -1) {
					perror("recv()");
					exit(EXIT_FAILURE);
				}
				printf("Reception de %d octets(s),  %d caractères \n", testRecv, nameSize);
				printf("nom du fichier : %s\n",pData[lastFreeId].fileList[i] );

			}
			lastFreeId++;
		}

		// ****************
		// * SEND FICHIER *
		// ****************

		// send nb client
		int testSend;
		testSend = send(sockCli, &nbPair, sizeof(int), 0);

		if(testSend == -1) {
			perror("send()");
			exit(EXIT_FAILURE);
		}


		// pour chaque client enregistrer
		for(int i = 0; i < nbPair; i++) {
			// send sockaddr_in
			testSend = send(sockCli, &pData[i].pair, sizeof(struct sockaddr_in), 0);
			if(testSend == -1) {
				perror("send()");
				exit(EXIT_FAILURE);
			}
			printf("nombre de fichier à send au client : %d\n", pData[i].nbFile);
			// send nb fichier
			int testSend = send(sockCli, &pData[i].nbFile, sizeof(int), 0);
			if(testSend == -1) {
				perror("send()");
				exit(EXIT_FAILURE);
			}

			
			// pour chaque fichier
			for(int j = 0; j < pData[i].nbFile; j++) {
				// send taille du nom
				int nameSize = strlen(pData[i].fileList[j])+1;
				testSend = mySendString(sockCli, pData[i].fileList[j], nameSize, 0);
				if(testSend <0){
					perror("mySendString");
					return -1;
				}
				else if (testSend==0){
					return 0;
				}
			}
		}
	}

	//Code mort ---> 
	printf("Fermeture de la socket serveur.....");
	int testCloseServ = close(sockServ);
	if(testCloseServ == -1) {
		perror("close()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	return 0;
}
