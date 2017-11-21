#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int myReceiv(int dsC) {
	int size, res;
	char message[10];
	if (res = recv(dsC, &size, sizeof(int), 0) < 0) {
		// send(dsC, -1, sizeof(int), 0);
		perror("taille_recv()");
		close(dsC);
		return -1;
	}
	printf("taille a recevoir : %d\n", size);
	while (size > 0) {
		res = recv(dsC, &message, size, 0);
		if (res < 0) {
			// send(dsC, -1, sizeof(int), 0);
			perror("message_recv()");
			close(dsC);
			return -1;
		}

		printf("%s\n", message); //action to_do

		size -= res;
	}

	return 0;
}


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

	struct sockaddr_in adClient;
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;

	int res = myReceiv(dSClient);
	if (res == -1) {
		printf("ERREUR myReceiv\n");
		close(dS);
		close(dSClient);
		return -1;
	}


	printf("\n");

	// res = send(dSClient, 0, sizeof(int), 0);

	close(dS);
	close(dSClient);
	return 0;
}
