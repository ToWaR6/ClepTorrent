#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <sys/types.h> // socket(), bind(), listen(), accept()
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()

struct pairData {
	char* ip[15];
	int* port;
	char* fileList[20];
};

int addClient(struct pairData* pairs, int* lastFreeId char* ip, int* port, char* fileList[20], int* nbMaxPair) {
	pairs->ip[lastFreeId] = ip;
	pairs->port[lastFreeId] = port;
	pairs->fileList[lastFreeId] = fileList;

	for(int i = 0; i < nbMaxPair; i++) {
		if(pairs->ip[i] == NULL) {
			*lastFreeId = i;
			return 0;
		}
	}

	return -1;
}

// Thread d'acceptation de nouveau client
void acceptPair() {

}

// Thread d'envoie de la structure a tous les clients
void sendStructure() {

}

int main(int argc, char const *argv[]) {
	if(argc != 2) {
		printf("Usage: %s <PORT>\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// variable du serveur
	struct pairData* pairs;
	int nbMaxPair = 3;
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
	if(testBind == -1) {
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

		// recv de la taille de la liste de noms
		// TODO

		char* fileList[20];

		// recv de la liste de noms
		// TODO

		// update de la structure
		char* ipCli;
		inet_ntop(AF_INET, addrCli.sin_addr, ipCli, sizeof(lenAddrCli));
			// test retour inet_pton
		int portCli = ntohs(addrCli.sin_port);
			// test retour ntohs

		printf("Add client to database.....");
		int testAddClient = addClient(pairs, &lastFreeId, ipCli, portCli, fileList, &nbMaxPair);
		if(testAddClient == -1) {
			printf("fail\n");
			perror("addClient()");
			exit(EXIT_FAILURE);
		}
		printf("done\n");

		// send de la taille de la structure
		// TODO

		// send de la structure
		// TODO

		// printf("Fermeture de la socket client.....");
		// int testCloseCli = close(sockCli);
		// if(testCloseCli == -1) {
		// 	printf("fail\n");
		// 	perror("close()");
		// 	exit(EXIT_FAILURE);
		// }
		// printf("done\n");
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