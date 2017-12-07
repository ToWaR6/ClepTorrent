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
	printf("Lancement du client\n");
	

	//Saisir le nom de fichier 
	char nomFichier[27] = "rsc/";
	printf("Saisir le nom du fichier que vous voulez envoyer ?\n");
	if(fgets(&nomFichier[4], 20, stdin)==NULL){
		perror("fgets nomFichier");
		return -1;
	}
	strtok(nomFichier, "\n");//Suppression du \n que reçoit fgets

	//Ouverture du fichier
	FILE* fp = fopen(nomFichier, "r");
	char q;
	while(fp==NULL){

		printf("Fichier non-trouvé\n 'q' pour quitter, 'entrée' pour continuer\n");
	 	q=getchar();
	 	if(q=='q'){
			return -1;
		}
		
		printf("Saisir le nom du fichier que vous voulez envoyer ?\n");
		if(fgets(&nomFichier[4], 20, stdin)==NULL){
			perror("fgets nomFichier");
			return -1;
		}
		strtok(nomFichier, "\n");
		printf("Recherche du fichier %s\n",nomFichier );
		fp = fopen(nomFichier,"r");
	}

	printf("\n===========================\n\n");
	//Create the socket
	int dS = socket(AF_INET, SOCK_STREAM , 0) ;
	if(dS == -1){
		perror("socket()");
		return -1;
	}

	printf("Socket créée\n");

	//Initialisation socket
	struct sockaddr_in sock; 
	sock.sin_family=AF_INET; //Family 
	sock.sin_port=htons(atoi(argv[2])); //Port
	if(strcmp(argv[1],"localhost")==0){
		if(inet_pton(AF_INET,"127.0.0.1",&sock.sin_addr)<0){//IP adress
			perror("inet_pton()");
			return -1;
		} 
		printf("Tentative de connexion au serveur 127.0.0.1:%s\n",argv[2] );
	}else{
		if(inet_pton(AF_INET,argv[1],&sock.sin_addr)<0){//IP adress
			perror("inet_pton()");
			return -1;
		} 
		printf("Tentative de connexion au serveur %s:%s\n",argv[1],argv[2] );
	}
	
	socklen_t tailleSock = sizeof(struct sockaddr_in); //Socket size
	//Connect to socket
	if(connect(dS,(struct sockaddr*)&sock,tailleSock)==-1){
		perror("connect()");
		close(dS);
		return -1;
	}
	printf("Vous êtes maintenant connecté au serveur\n\n");
	



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
	
	int tailleNomFichier = strlen(nomFichier)+1;//Pour envoyer le \0
	printf("\nVous envoyez le fichier %s\n",nomFichier);
	printf("Ce fichier est de la taille %f MO\n\n",(double)tailleF/1048576);

	if(mySendFile(dS,fp,(int)tailleF,nomFichier,tailleNomFichier)==-1){
		perror("mySendFile()");
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
	printf("Fichier envoyé\n Fermeture du client\n");
	return 0;
}
