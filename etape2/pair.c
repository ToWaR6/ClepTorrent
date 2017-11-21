#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <sys/types.h> // socket(), connect()
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()

int main(int argc, char const *argv[]) {
	if(argc != 3) {
		printf("Usage: %s <IP_SERV> <PORT>\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	printf("Creation de la socket.....");
	int sockCli = socket(AF_INET, SOCK_STREAM, 0);
	if(sockCli == -1) {
		printf("fail\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	struct sockaddr_in addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &addrServ.sin_addr);

	printf("Conenction au serveur.....");
	int testConnect = connect(sockCli, (struct sockaddr*)&addrServ, sizeof(addrServ));
	if(testConnect == -1) {
		printf("fail\n");
		perror("connect()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	printf("Fermeture de la socket client.....");
	int testClose = close(sockCli);
	if(testClose == -1) {
		printf("fail\n");
		perror("close()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	return 0;
}