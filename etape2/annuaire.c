#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <sys/types.h> // socket(), bind(), listen(), accept()
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()

int main(int argc, char const *argv[]) {
	struct sockaddr_in addrsClient[3];
	int lastFreeId = 0;

	if(argc != 2) {
		printf("Usage: %s <PORT>\n", argv[1]);
		exit(EXIT_FAILURE);
	}

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

		addrsClient[lastFreeId] = addrCli;
		lastFreeId++;

		//envoie liste addr
		if(send(dS,&tailleF,sizeof(int),0)==-1){
			perror("send() ");
			return(-1);
		}

		printf("Fermeture de la socket client.....");
		int testCloseCli = close(sockCli);
		if(testCloseCli == -1) {
			printf("fail\n");
			perror("close()");
			exit(EXIT_FAILURE);
		}
		printf("done\n");
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