#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sock_addr
#include <sys/socket.h>
#include <arpa/inet.h> //htons, inet_pton
#include <unistd.h> //close
#include <errno.h>
#include <sys/stat.h>//Taille fichier entre autre
#include <string.h>
#include "functionFile.h"

int main(int argc, char const *argv[]){
	if(argc<3){
		printf("%s -ip -port\n", argv[0]);
		return -1;
	}

	//Create the socket
	int dS = socket(AF_INET, SOCK_STREAM , 0) ;
	if(dS == -1){
		perror("socket()");
		return -1;
	}
	//Define socket and size
	struct sockaddr_in sock; 
	sock.sin_family=AF_INET; //Family 
	sock.sin_port=htons(atoi(argv[2])); //Port
	if(inet_pton(AF_INET,argv[1],&sock.sin_addr)==-1){//IP adress
		perror("inet_pton()");
		return -1;
	} 

	socklen_t tailleSock = sizeof(struct sockaddr_in); //Socket size
	//Connect to socket
	if(connect(dS,(struct sockaddr*)&sock,tailleSock)==-1){
		perror("connect()");
		close(dS);
		return -1;
	}
	printf("Envoi du fichier\n");
	
	printf("Socket connectée\n");
	//Saisir le nom de fichier 
	char nomFichier[27] = "rsc/";
	printf("Saisir le nom du fichier que vous voulez envoyer ?\n");
	if(fgets(&nomFichier[4], sizeof(nomFichier), stdin)==NULL){
		perror("fgets nomFichier");
		return -1;
	}
	strtok(nomFichier, "\n");
	//Ouverture du fichier
	FILE* fp = fopen(nomFichier, "r");
	char q;
	while(fp==NULL){

		printf("Fichier non-trouvé, q pour quitter\n");
	 	q=getchar();
	 	if(q=='q'){
	 		if(close(dS)){
				perror("close() ");
				return -1;
			}
			return -1;
		}
		strtok(nomFichier, "\n");
		
		printf("Saisir le nom du fichier que vous voulez envoyer ?\n");
		if(fgets(&nomFichier[4], 20, stdin)==NULL){
			perror("fgets nomFichier");
			return -1;
		}
		strtok(nomFichier, "\n");
		printf("Recherche du fichier %s\n",nomFichier );
		fp = fopen(nomFichier,"r");
		

	}

	//Calcul taille du fichier
	int tailleF;
	struct stat st;
	if (stat(nomFichier, &st) == 0)
		tailleF = st.st_size;
	else{
		perror("stat() (size)");
		close(dS);
		fclose(fp);
		return -1;
	}
	
	printf("%d octet(s) à envoyer\n", tailleF);
	int tailleNomFichier = strlen(nomFichier)+1;
	
	printf("Vous envoyez le fichier %s (%d charactère(s))\n",nomFichier,tailleNomFichier );
	printf("Ce fichier est de la taille %f MO\n",(double)tailleF/1048576);
	if(mySend(dS,fp,(int)tailleF,nomFichier,tailleNomFichier)==-1){
		perror("mySend()");
		fclose(fp);
		close(dS);
		return -1;
	}

	if(fclose(fp)!=0){
		perror("fclose()");
		close(dS);
		return -1;
	}
	if(close(dS)){
		perror("close() ");
		return -1;
	}
	printf("Fichier envoyé\n");
	return 0;
}