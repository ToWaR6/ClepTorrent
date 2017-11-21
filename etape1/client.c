#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sock_addr
#include <sys/socket.h>
#include <arpa/inet.h> //htons, inet_pton
#include <unistd.h> //close
#include <errno.h>

int mySend(int sockfd,const void *buf,size_t len){
	int snd =0;
	int tmp = 0;
	int rest= len;
	char *ptr = (char*) buf;
	while(snd<len){
		tmp= send(sockfd,&ptr[snd],rest,0);
		if(snd==-1){
			perror("send() ");
			return(-1);
		}else{
			snd+=tmp;
			rest-=tmp;
		}
	}
	return 0;
}

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
	int tailleF = 6;
	if(send(dS,&tailleF,sizeof(int),0)==-1){
		perror("send() ");
		return(-1);
	}

	char *buff="Hello";
	mySend(dS,buff,tailleF);//Envoie le fichier (boucle)

	if(close(dS)){
		perror("close() ");
		exit(-1);
	}else{
		printf("dS bien fermé\n");
	}
	return 0;
}