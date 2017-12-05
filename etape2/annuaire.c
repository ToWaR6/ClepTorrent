#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <string.h> // strlen()
#include <sys/types.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <sys/socket.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()

struct pairData {
	struct sockaddr_in pair;
	int nbFile;
	char** fileList;
};

int main(int argc, char const *argv[]) {
	if(argc != 3) {
		printf("Usage: %s <PORT> <PAIR_MAX>\n", argv[0]);
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
		struct sockaddr_in addrCli;
		socklen_t lenAddrCli;

		printf("Accept de la socket.....");
		int sockCli = accept(sockServ, (struct sockaddr*)&addrCli, &lenAddrCli);
		if(sockCli == -1) {
			printf("fail\n");
			perror("accept()");
			exit(EXIT_FAILURE);
		}
		printf("done\n");

		struct sockaddr_in addrCli2 = addrCli;

		// ****************
		// * RECV FICHIER *
		// ****************

		// recv du port
		printf("Reception du port d'ecoute du client.....");
		short port = 0;
		int testRecv = recv(sockCli, &port, sizeof(short), 0);
		if(testRecv == -1) {
			printf("fail\n");
			perror("recv()");
			exit(EXIT_FAILURE);
		}
		// printf("done\n");
		printf("Reception de %d bit, pour la valeur %d\n", testRecv, port);

		addrCli2.sin_port = port;
		pData[lastFreeId].pair = addrCli2;

		// recv du nombre de fichier
		printf("Reception du nombre de fichier du client.....");
		int nbFile = 0;
		testRecv = recv(sockCli, &nbFile, sizeof(int), 0);
		if(testRecv == -1) {
			printf("fail\n");
			perror("recv()");
			exit(EXIT_FAILURE);
		}
		// printf("done\n");
		printf("Reception de %d bit, pour la valeur %d\n", testRecv, nbFile);

		pData[lastFreeId].nbFile = nbFile;
		pData[lastFreeId].fileList = (char**)malloc(nbFile * sizeof(char*));

		// for charque fichier
		for(int i = 0; i < nbFile; i++) {
			// recv la taille du nom
			printf("Reception de la taille du nom du fichier %d.....", i);
			int nameSize = 0;
			testRecv = recv(sockCli, &nameSize, sizeof(int), 0);
			if(testRecv == -1) {
				printf("fail\n");
				perror("recv()");
				exit(EXIT_FAILURE);
			}
			// printf("done\n");
			printf("Reception de %d bit, pour la valeur %d\n", testRecv, nameSize);

			pData[lastFreeId].fileList[i] = (char*)malloc(nameSize * sizeof(char));

			// recv nom
			printf("Reception du nom du fichier %d.....", i);
			char name[nameSize];
			testRecv = recv(sockCli, &name, nameSize, 0);
			if(testRecv == -1) {
				printf("fail\n");
				perror("recv()");
				exit(EXIT_FAILURE);
			}
			// printf("done\n");
			printf("Reception de %d bit, pour la valeur %s\n", testRecv, name);

			pData[lastFreeId].fileList[i] = name;
		}

		nbPair++;

		// ****************
		// * SEND FICHIER *
		// ****************

		// send nb client
		printf("Envoie du nombre de client.....");
		int testSend = send(sockCli, &nbPair, sizeof(int), 0);
		if(testSend == -1) {
			printf("fail\n");
			perror("send()");
			exit(EXIT_FAILURE);
		}
		printf("done\n");

		// pour chaque client enregistrer
		for(int i = 0; i < nbPair; i++) {
			// send sockaddr_in
			printf("Envoie du sockaddr_in client.....");
			testSend = send(sockCli, &pData[i].pair, sizeof(sockaddr_in), 0);
			if(testSend == -1) {
				printf("fail\n");
				perror("send()");
				exit(EXIT_FAILURE);
			}
			printf("done\n");

			// send nb fichier
			printf("Envoie du nombre de fichier du client.....");
			int testSend = send(sockCli, &pData[i].nbFile, sizeof(int), 0);
			if(testSend == -1) {
				printf("fail\n");
				perror("send()");
				exit(EXIT_FAILURE);
			}
			printf("done\n");

			// pour chaque fichier
			for(int j = 0; j < pData[i].nbFile; j++) {
				// send taille du nom
				printf("Envoie de la taille du fichier %d.....", j);
				int nameSize = strlen(pData[i].fileList[j])+1;
				testSend = send(sockCli, &nameSize, sizeof(int), 0);
				if(testSend == -1) {
					printf("fail\n");
					perror("send()");
					exit(EXIT_FAILURE);
				}
				printf("done\n");

				// send le nom
				printf("Envoie du nom de fichier %d.....", j);
				testSend = send(sockCli, &pData[i].fileList[j], nameSize, 0);
				if(testSend == -1) {
					printf("fail\n");
					perror("send()");
					exit(EXIT_FAILURE);
				}
				printf("done\n");
			}
		}
		
		// lastFreeId = -1;
		// for(int i = 0; i < nbMaxPair; i++) {
		// 	if(pData.pairs[i] == NULL) {
		// 		lastFreeId = i;
		// 	}
		// }
	}

	printf("Fermeture de la socket serveur.....");
	int testCloseServ = close(sockServ);
	if(testCloseServ == -1) {
		printf("fail\n");
		perror("close()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	return 0;
}
