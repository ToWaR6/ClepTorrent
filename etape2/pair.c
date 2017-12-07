#include <stdio.h> // perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // close()
#include <errno.h> // errno
#include <sys/types.h> // socket(), connect()
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_pton()
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include "functionFile.h"
/*
1) de  se  connecter  au  serveur,  
1-bis ) PORT htons 
2)de transmettre à ce dernier la liste des fichiers dont il dispose pour téléchargement par
d’autres pairs,
3) de récupérer la liste des pairs connectés au réseau ainsi que la liste des
fichiers disponibles sur chaque pair distant,
4) de se connecter à un autre pair
5) de se déconnecter  de  ce  dernier,
6)  de  quitter  le  réseau  ou  de  reprendre  au  point  
3)  pour avoir  accès  à  une  éventuelle  mise  à  jour  de  la  liste  des  pairs  présent
*/


struct pairData {
	struct sockaddr_in pairs;
	int nbFiles;
	char** fileList;
};

struct paramsThreadClient{
	int port;
	struct sockaddr_in *sockAddr;
	char dest[256];
	int nbPair;
	struct pairData* tabClient;

};
/*Boucle de reception annuaire 
	Je vais recevoir x clients
	Je créer un tableau de la structure de taille pair_data de taille x
	Je boucle sur x
		Je reçois y le nombre de fichier du client x(n)
		Je reçois sock_x(n) la socket du client x_(n) que je stocke
		Je crée un tableau de fileList de taille y 
		Je boucle sur y
			Je reçois la t taille du nom de fichier
			Je le stocke à l'index y(n) de taille t
*/

//argument : port d'ecoute 
void* serverThread(void* arg) {

	int port = *((int*) arg);

	int dS = socket(AF_INET, SOCK_STREAM, 0);
	if (dS < 0) {
		perror("socket()");
		pthread_exit(NULL);
	}
	printf("SERVER : Socket ecoute créée\n");
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = (short) htons(port);

	int err = bind(dS, (struct sockaddr*) &ad, sizeof(ad));
	if (err < 0) {
		perror("bind()");
		close(dS);
		pthread_exit(NULL);
	}

	err = listen(dS, 2);
	if (err < 0) {
		perror("listen()");
		close(dS);
		pthread_exit(NULL);
	}


	// definition var pour le while 1
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient, sizeName, res, tailleF;
	char nomFichier[27] = "rsc/"; //------------------- nom dossier en dur dans le code a modifier plus tard ---------------------
	struct sockaddr_in adClient;
	struct stat st;

	while (1) {
		printf("SERVER : attente de client\n");
		dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;
		if (dSClient < 0) {
			perror("accept ");
			close(dS);
			pthread_exit(NULL);
		}
		printf("Un nouveau client !\n");
		if ((err = recv(dSClient, &sizeName, sizeof(int), 0)) < 0) {
			perror("recv() taille");
			close(dS);
			close(dSClient);
			pthread_exit(NULL);
		}
		if ((err = myLoopReceiv(dSClient, &nomFichier[4], sizeName, 0)) < 0) {
			perror("name_recv()");
			close(dS);
			close(dSClient);
			pthread_exit(NULL);
		}

		FILE* fp = fopen(nomFichier, "r");
		if (fp == NULL) {
			printf("ya pas de fichier %s...\n", nomFichier);
			close(dSClient);
		} else {
				if (stat(nomFichier, &st) == 0)
					tailleF = st.st_size;
				else{
					perror("stat() (size)");
					close(dS);
					close(dSClient);
					fclose(fp);
					pthread_exit(NULL);
				}
			
			if ((err = mySendFile(dSClient, fp, tailleF, nomFichier, sizeName)) < 0) {
				perror("mySendFile()");
				close(dS);
				close(dSClient);
				pthread_exit(NULL);
			}

			printf("envoyé\n");
			fclose(fp);
			close(dSClient);
		}

	}
	

}

void *clientThread(void* arg){

	struct paramsThreadClient* pT  = (struct paramsThreadClient*) arg;
	struct sockaddr_in * addrServ = pT->sockAddr;
	int port = pT->port;
	int reponse = 0;
	int resultScan = 0;
	char nomFichier[128];
	char destination[512];
	strcpy(destination,pT->dest);
	destination[strlen(pT->dest)] = '/';
	//m-aj variables
	int sockAnnuaire,res,nbClient,tailleNom;
	nbClient = pT->nbPair;
	struct pairData *tabClient = pT->tabClient;

	do{
		printf("Bonjour voulez vous mettre-à-jour (0), télécharger un fichier (1) ou éteindre le P2P (2)\n");
		resultScan = scanf("%d",&reponse);
		if(resultScan==EOF){
			perror("scanf réponse\n");
			pthread_exit(NULL);
		}
		if(resultScan==0){
			while(fgetc(stdin)!='\n');
		}
		if(reponse==0){//M-a-j
			printf("Creation de la socket.....\n");
			sockAnnuaire = socket(AF_INET, SOCK_STREAM, 0);
			if(sockAnnuaire == -1) {
				perror("socket()");
				pthread_exit(NULL);
			}
			if(connect(sockAnnuaire, (struct sockaddr*)addrServ, sizeof(*addrServ)) == -1) {
				perror("connect()");
				pthread_exit(NULL);
			}
			printf("Connecté au serveur\n");
			if((res = send (sockAnnuaire,&port,sizeof(short),0))<0){
				perror("send() port");
				pthread_exit(NULL);
			}
			printf("Port %d envoyé \n",port);

			if((res = recv(sockAnnuaire, &nbClient, sizeof(int), 0)) < 0) {
				perror("recv nbClient");
				pthread_exit(NULL);
			}
			printf("Nombre de client : %d\n",nbClient );
			struct pairData tabClient[nbClient];
			for (int i = 0; i < nbClient; i++){
				if ((res = recv(sockAnnuaire, &tabClient[i].pairs, sizeof(struct sockaddr_in), 0)) < 0) {
					perror("recv pairs");
					pthread_exit(NULL);
				}
				if((res = recv(sockAnnuaire, &tabClient[i].nbFiles, sizeof(int), 0)) < 0) {
					perror("recv nbFiles");
					pthread_exit(NULL);
				}
				printf("Nombre de fichiers %d du client %d \n",tabClient[i].nbFiles,i );
				tabClient[i].fileList = (char**)malloc(tabClient[i].nbFiles * sizeof(char*));
				for (int j = 0; j < tabClient[i].nbFiles; j++){
					if ((res = recv(sockAnnuaire, &tailleNom, sizeof(int), 0)) < 0) {
						perror("recv tailleNom");
						pthread_exit(NULL);
					}
					printf("Nombre de caractères du fichier %d : %d\n",j,tailleNom );
					tabClient[i].fileList[j] = (char*)malloc(tailleNom * sizeof(char));
					if((res = myLoopReceiv(sockAnnuaire, tabClient[i].fileList[j], tailleNom, 0)) < 0) {
						perror("recv nomFichier");
						pthread_exit(NULL);
					}
					else if(res==0){
						pthread_exit(NULL);
					}
					printf("fichier[%d] :%s\n", j,tabClient[i].fileList[j]);
				}
			}
			if(close(sockAnnuaire) == -1) {
				perror("close()");
				pthread_exit(NULL);
			}
		}
		else if(reponse == 1){//client

			printf("Saisir le nom du fichier que vous voulez recevoir ?\n");
			if((resultScan = scanf(" %20s",nomFichier))==EOF){
				perror("scanf nomFichier");
				pthread_exit(NULL);
			}
			if(resultScan==0){
				while(fgetc(stdin)!='\n');
			}
			strtok(nomFichier, "\n");
			nomFichier[strlen(nomFichier)] = '\0';
			strcat(destination,nomFichier);
			printf("Vous reclamez le fichier %s il sera stocké à l'adresse %s \n",nomFichier,destination );

			printf("Quel est le numero du client qui possède ce fichier ? \n");
			resultScan = scanf("%d",&reponse);
			if(resultScan==EOF){
				perror("scanf réponse\n");
				pthread_exit(NULL);
			}
			if(resultScan==0){
				while(fgetc(stdin)!='\n');
			}
			if(reponse<nbClient){
				int sockPair = socket(AF_INET, SOCK_STREAM, 0);
				if(sockPair == -1) {
					printf("fail\n");
					perror("socket()");
					pthread_exit(NULL);
				}
				printf("Socket client crée\n");

				printf("Connexion au serveur.....");
				if(connect(sockPair, (struct sockaddr*)&tabClient[reponse], sizeof(tabClient[reponse])) == -1) {
					perror("connect()");
					pthread_exit(NULL);
				}
				res=mySendString(sockPair,nomFichier,strlen(nomFichier)+1,0);
				if(res<0){
					perror("mySendString");
					pthread_exit(NULL);
				}
				else if (res==0){
					pthread_exit(NULL);
				}

				res = myReceivFile(sockPair,destination);
				if (res == -1) {
					perror("ERREUR myReceivFile");
					close(sockPair);
					pthread_exit(NULL);
				}
				else if (res ==0){
					printf("Le pair ne semble pas possèder le client désolé\n");
				}
				else{
					printf("Fichier bien reçu\n");
				}
				strcpy(destination,pT->dest);
			}else{
				printf("Il n'y a pas autant de client\n");
			}
			reponse = 1;
		}
	}while(reponse!=2);
	pthread_exit(NULL);

}
int main(int argc, char const *argv[]) {
	if(argc < 5) {
		printf("Usage: %s <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossierSource> <dossierDest> \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//Recupération des fichiers du pair:
	DIR* rep = NULL;
	rep = opendir(argv[4]); 
	if (rep == NULL){
		perror("opendir()");
		exit(-1); 
	}
	printf("Dossier ouvert\n");
	struct dirent* fichierLu;

	//Compte nombre fichier
	int cptFiles = 0;
	while((fichierLu = readdir(rep))!=NULL){
		if(strcmp(fichierLu->d_name,".") != 0 &&  strcmp(fichierLu->d_name,"..") != 0)
			cptFiles++;
	}

	char fileList[cptFiles][256];//256 car max de dirent
	rewinddir(rep);
	int index = 0;
	printf("Le dossier contient %d fichiers :\n",cptFiles);
	while((fichierLu = readdir(rep))!=NULL){
		if(strcmp(fichierLu->d_name,".") != 0 &&  strcmp(fichierLu->d_name,"..") != 0){
			strcpy(fileList[index], fichierLu->d_name);
			printf("- %s\n",fileList[index] );
			index++;
		}
			
	}
	if (closedir(rep) == -1){
		perror("closedir()");
		exit(-1);
	}
	//Fin de la récuperation des fichiers du pair

	printf("Creation de la socket.....");
	int sockAnnuaire = socket(AF_INET, SOCK_STREAM, 0);
	if(sockAnnuaire == -1) {
		printf("fail\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");

	struct sockaddr_in addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = (short) htons(atoi(argv[2]));

	if(strcmp(argv[1],"localhost")==0){
		if(inet_pton(AF_INET,"127.0.0.1",&addrServ.sin_addr)<0){//IP adress
			perror("inet_pton()");
			return -1;
		} 
		printf("Tentative de connexion au serveur 127.0.0.1:%s\n",argv[2] );
	}else{
		if(inet_pton(AF_INET,argv[1],&addrServ.sin_addr)<0){//IP adress
			perror("inet_pton()");
			return -1;
		} 
		printf("Tentative de connexion au serveur %s:%s\n",argv[1],argv[2] );
	}

	printf("Connexion au serveur.....");
	int testConnect = connect(sockAnnuaire, (struct sockaddr*)&addrServ, sizeof(addrServ));
	if(testConnect == -1) {
		printf("fail\n");
		perror("connect()");
		exit(EXIT_FAILURE);
	}
	printf("Connecté au serveur\n");

	int res;
	short port = (short) htons(atoi(argv[3]));
	//Envoie du fichier
	if((res = send (sockAnnuaire, &port,sizeof(short),0))<0){
		perror("send() port");
		return -1;
	}

	// printf("Port %d envoyé \n",ad.sin_port);

	//Envoie des fichiers du pair
	if((res=send(sockAnnuaire,&cptFiles,sizeof(int),0))<0){ 
		perror("send() taille");
		return -1;
	}else{
		printf("\nLe premier send a envoyé : %d octet(s)\n", res);
		printf("ça correspond à %d fichier(s)\n\n",cptFiles );
	}
	int tailleNom = 0;
	for (int i = 0; i < cptFiles; ++i){
		fileList[i][strlen(fileList[i])]= '\0';
		tailleNom = strlen(fileList[i])+1;

		res=mySendString(sockAnnuaire,fileList[i],tailleNom,0);
		if(res<0){
			perror("mySendString");
			return res;
		}
		else if (res==0){
			return res;
		}
	}
	/*Boucle de reception annuaire 
		Je vais recevoir x clients
		Je créer un tableau de la structure de taille pair_data de taille x
		Je boucle sur x
			Je reçois y le nombre de fichier du client x(n)
			Je reçois sock_x(n) la socket du client x_(n) que je stocke
			Je crée un tableau de fileList de taille y 
			Je boucle sur y
				Je reçois la t taille du nom de fichier
				Je le stocke à l'index y(n) de taille t
	*/

	int nbClient;
	if((res = recv(sockAnnuaire, &nbClient, sizeof(int), 0)) < 0) {
		perror("recv nbClient");
		return -1;
	}
	printf("Nombre de client : %d\n",nbClient );
	struct pairData tabClient[nbClient];
	for (int i = 0; i < nbClient; i++){
		if ((res = recv(sockAnnuaire, &tabClient[i].pairs, sizeof(struct sockaddr_in), 0)) < 0) {
			perror("recv pairs");
			return -1;
		}
		if((res = recv(sockAnnuaire, &tabClient[i].nbFiles, sizeof(int), 0)) < 0) {
			perror("recv nbFiles");
			return -1;
		}
		printf("Nombre de fichiers %d \n",tabClient[i].nbFiles );
		tabClient[i].fileList = (char**)malloc(tabClient[i].nbFiles * sizeof(char*));
		for (int j = 0; j < tabClient[i].nbFiles; j++){
			printf("%d\n", tabClient[i].nbFiles);
			if ((res = recv(sockAnnuaire, &tailleNom, sizeof(int), 0)) < 0) {
				perror("recv tailleNom");
				return -1;
			}
			printf("Nombre de caractères du fichier %d : %d\n",j,tailleNom );
			tabClient[i].fileList[j] = (char*)malloc(tailleNom * sizeof(char));
			if((res = recv(sockAnnuaire, tabClient[i].fileList[j], tailleNom, 0)) < 0) {
				perror("recv nomFichier");
				return -1;
			}
			else if(res ==0){
				return 0;
			}
			printf("fichier[%d] :%s\n", j,tabClient[i].fileList[j]);
		}
	}

	printf("Fermeture de la socket pair.....");
	int testClose = close(sockAnnuaire);
	if(testClose == -1) {
		printf("fail\n");
		perror("close()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");
	
	pthread_t tListen1;
	pthread_t tListen2;
	int portParam = atoi(argv[3]);
	pthread_create(&tListen1, NULL, &serverThread, &portParam);

	struct paramsThreadClient pT;
	pT.port = htons(portParam);
	pT.sockAddr = &addrServ;
	strcpy(pT.dest,argv[5]);
	pT.nbPair = nbClient;
	pT.tabClient = tabClient;

	pthread_create(&tListen2,NULL,&clientThread,&pT);
	// lancer serverThread
	// arg : argv[3]

	pthread_join(tListen1, NULL);
	pthread_join(tListen2, NULL);
	return 0;
}

