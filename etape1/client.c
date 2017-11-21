#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sock_addr
#include <sys/socket.h>
#include <arpa/inet.h> //htons, inet_pton
#include <unistd.h> //close
#include <errno.h>
#include <sys/stat.h>//Taille fichier entre autre
#include "functionFile.h"

int main(int argc, char const *argv[]){
	if(argc<3){
		printf("%s -ip -port\n", argv[0]);
		exit(-1);
	}

	//Create the socket
	int dS = socket(AF_INET, SOCK_STREAM , 0) ;
	if(dS == -1){
		perror("socket() ");
     	exit(-1);
	}
	//Define socket and size
	struct sockaddr_in sock; 
	sock.sin_family=AF_INET; //Family 
	sock.sin_port=htons(atoi(argv[2])); //Port
	if(inet_pton(AF_INET,argv[1],&sock.sin_addr)==-1){//IP adress
		perror("inet_pton()");
		exit(-1);
	} 

	socklen_t tailleSock = sizeof(struct sockaddr_in); //Socket size
	//Connect to socket
	if(connect(dS,(struct sockaddr*)&sock,tailleSock)==-1){
		perror("connect() ");
		close(dS);
		exit(-1);
	}
	printf("Connect to ClepTorrent\n");
	
	//Demander quel fichier envoyer
	//Recupere le fichier
	//Calculer taille fichier
	//Envoyer taille
	//Boucler tant qu'il reste des octets
		//Envoie le fichier 
	//Fini
	char *filename = "rsc/16MO.txt";
	FILE* fp = fopen(filename, "r+");
	if(fp==NULL){
		perror("Ouverture fichier");
		close(dS);
		exit(-1);
	}

	int tailleF;
	struct stat st;
	if (stat(filename, &st) == 0)
		tailleF = st.st_size;
    else{
		perror("stat (size)");
		close(dS);
		fclose(fp);
		exit(-1);
	}
    printf("%d\n",tailleF );
	mySendFile(dS,fp,tailleF);

	fclose(fp);
	if(close(dS)){
		perror("close() ");
		fclose(fp);
		exit(-1);
	}else{
		printf("dS bien fermé\n");
	}
	return 0;
}