#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sock_addr
#include <sys/socket.h>
#include <arpa/inet.h> //htons, inet_pton
#include <unistd.h> //close
#include <errno.h>

int main(int argc, char const *argv[])
{
	if(argc<3){
		printf("%s -ip -port\n", argv[0]);
		exit(-1);
	}

	//Création de la socket
	int dS = socket(AF_INET, SOCK_STREAM , 0) ;
	if(dS == -1){
		perror("socket() ");
     	exit(-1);
	}
	//Definition socket et taille
	struct sockaddr_in sock; 
	sock.sin_family=AF_INET; //Famille 
	sock.sin_port=htons(atoi(argv[1])); //Numero de PORT
	if(inet_pton(AF_INET,argv[2],&sock.sin_addr)==-1){
		perror("inet_pton()");
		exit(-1);
	} 
	//Adresse IP
	socklen_t tailleSock = sizeof(struct sockaddr_in); //Taille de la socket

	if(connect(dS,(struct sockaddr*)&sock,tailleSock)==-1){
		perror("connect() ");
		exit(-1);
	}
	if(close(dS)){
		perror("close() ");
		exit(-1);
	}else{
		printf("dS bien fermé\n");
	}
	return 0;
}