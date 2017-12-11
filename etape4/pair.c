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


// structure pour stocker les informations de connexion d'un client
struct pairData {
	struct sockaddr_in pairs; // adresse et port du pair
	int nbFiles; // nombre de fichier qu'il possède
	char** fileList; // liste des nom de fichier
};

// structure pour les parametres passe au thread "client"
struct paramsThreadClient{
	int port; // port d'ecoute du serveur annuaire
	struct sockaddr_in *sockAddr; // adresse du serveur annuaire
	char dest[256]; // chemin de destination des fichier telecharger
	int nbPair; // nombre de pairs connectes
	struct pairData* tabClient; // tableau de donnees de tous les pairs connectes
};

// structure pour les parametres passe au thread d'envoi de fichier
struct paramsThreadSendFile{
	int nbFiles; // nombre de fichier et de mutex du client
	pthread_mutex_t* tabMutex; // tableau de mutex (1 pour chaque fichier)
	char rsc[256]; // chemin du dossier de ressource
	int dSClient; // descripteur de socket où envoyer les fichiers
	char **fileList; // liste des fichiers que le client possède
};

// structure pour les parametres passe au thread "serveur"
struct paramsThreadServer{
	int port; // port d'écoute du thread server
	struct paramsThreadSendFile paramsSendFile; // parametre du thread d'envoi de fichier
};

// Thread d'envoi de fichier
void *sendFileThread(void* arg) {
	struct paramsThreadSendFile* ps  = (struct paramsThreadSendFile*) arg;
	int dSClient = ps->dSClient; 


	char pathFichier[256]; // chemin du fichier à envoyer (seulement dossier)
	strcpy(pathFichier,ps->rsc); 
	struct stat st;
	int sizeName, tailleF;

	// reception de la taille du nom du fichier
	if (recv(dSClient, &sizeName, sizeof(int), 0) < 0) {
		perror("recv() taille");
		close(dSClient);
		pthread_exit(NULL);
	}

	char nomFichier[sizeName]; // nom du fichier
	// reception du nom du fichier
	if (myLoopReceiv(dSClient, nomFichier, sizeName, 0) < 0) {
		perror("name_recv()");
		close(dSClient);
		pthread_exit(NULL);
	}

	// si le chemin ne se termine pas par '/' le rajouter
	if (pathFichier[strlen(pathFichier)-1] != '/') {
		strcat(pathFichier, "/");
	}
	strcat(pathFichier, nomFichier);

	// recherche de l'index du fichier dans la liste des fichiers de l'utilisateur
	int indexMutex;
	for (int i = 0; i < ps->nbFiles; i++) {
		printf("ps->fileList[i] : %s\n", ps->fileList[i]);
		if (strcmp(ps->fileList[i], nomFichier)==0) {
			indexMutex = i;
			break;
		}
	}

	// lock mutex correspondant au fichier en demande de téléchargement
	pthread_mutex_lock(&(ps->tabMutex[indexMutex]));

	FILE* fp = fopen(pathFichier, "r"); // ouverture du fichier en lecture
	if (fp == NULL) {
		printf("il n'y a pas de fichier '%s'...\n", pathFichier);
		close(dSClient);
	} else {
		if (stat(pathFichier, &st) == 0)
			tailleF = st.st_size; // recuperation de la taille du fichier
		else{
			perror("stat() (size)");
			close(dSClient);
			fclose(fp);
			pthread_exit(NULL);
		}
		
		// envoi du fichier
		if (mySendFile(dSClient, fp, tailleF, pathFichier, sizeName) < 0) {
			perror("mySendFile()");
			close(dSClient);
			pthread_exit(NULL);
		}

		printf("envoyé\n");
		fclose(fp);
		close(dSClient);
	}

	pthread_mutex_unlock(&(ps->tabMutex[indexMutex]));
}


// Thread server
void* serverThread(void* arg) {
	struct paramsThreadServer* pS  = (struct paramsThreadServer*) arg;
	struct paramsThreadSendFile pts = pS->paramsSendFile;
	int port = pS->port;
	int dS = socket(AF_INET, SOCK_STREAM, 0);
	if (dS < 0) {
		perror("socket()");
		// pthread_exit(NULL);
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = (short) htons(port);

	int err = bind(dS, (struct sockaddr*) &ad, sizeof(ad));
	if (err < 0) {
		perror("bind()");
		close(dS);
		// pthread_exit(NULL);
		exit(EXIT_FAILURE);
	}

	err = listen(dS, 2);
	if (err < 0) {
		perror("listen()");
		close(dS);
		// pthread_exit(NULL);
		exit(EXIT_FAILURE);
	}


	// definition var pour le while 1
	socklen_t soA = sizeof(struct sockaddr_in);
	int dSClient, sizeName, res;
	struct sockaddr_in adClient;

	while (1) {
		dSClient = accept(dS, (struct sockaddr *) &adClient, &soA) ;
		if (dSClient < 0) {
			perror("accept ");
			close(dS);
			pthread_exit(NULL);
		}

		pts.dSClient = dSClient;
		pthread_t threadPair;
		if(pthread_create(&threadPair, NULL, &sendFileThread, &pts)!=0){
			perror("pthread_create - sendFileThread");
			return NULL;
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
	if (destination[strlen(pT->dest)-1] != '/') {
		// destination[strlen(pT->dest)] = '/';
		strcat(destination, "/");
	}
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
			tabClient = (struct pairData*)malloc(nbClient * sizeof(struct pairData*));
			for (int i = 0; i < nbClient; i++){
				if ((res = recv(sockAnnuaire, &tabClient[i].pairs, sizeof(struct sockaddr_in), 0)) < 0) {
					perror("recv pairs");
					pthread_exit(NULL);
				}
				if((res = recv(sockAnnuaire, &tabClient[i].nbFiles, sizeof(int), 0)) < 0) {
					perror("recv nbFiles");
					pthread_exit(NULL);
				}
				printf("Le client %d possède %d fichier(s) \n",i,tabClient[i].nbFiles );
				tabClient[i].fileList = (char**)malloc(tabClient[i].nbFiles * sizeof(char*));
				for (int j = 0; j < tabClient[i].nbFiles; j++){
					if ((res = recv(sockAnnuaire, &tailleNom, sizeof(int), 0)) < 0) {
						perror("recv tailleNom");
						pthread_exit(NULL);
					}
					tabClient[i].fileList[j] = (char*)malloc(tailleNom * sizeof(char));
					if((res = myLoopReceiv(sockAnnuaire, tabClient[i].fileList[j], tailleNom, 0)) < 0) {
						perror("recv nomFichier");
						pthread_exit(NULL);
					}
					else if(res==0){
						pthread_exit(NULL);
					}
					printf("\tfichier[%d] :%s\n", j,tabClient[i].fileList[j]);
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
			printf("Vous reclamez le fichier '%s' il sera stocké à l'adresse %s \n",nomFichier,destination );

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
				printf("Socket client crée, client sur port %d\n", ntohs(tabClient[reponse].pairs.sin_port));

				printf("Connexion au client.....\n");
				if(connect(sockPair, (struct sockaddr*)&(tabClient[reponse].pairs), sizeof(tabClient[reponse].pairs)) == -1) {
					// perror("connect()");
					// pthread_exit(NULL);
					printf("Connexion refusé\n");
				} else {
					printf("Client connecté.....\n");
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
						printf("Le pair ne semble pas posséder le fichier... désolé\n");
					}
					else{
						printf("Fichier bien reçu\n");
					}
					strcpy(destination,pT->dest);
					destination[strlen(pT->dest)] = '/';
				}
			}else{
				printf("Il n'y a pas autant de client\n");
			}
			memset(destination,'\0',512);
			reponse = 1;
		}
	}while(reponse!=2);
	pthread_exit(NULL);

}
int main(int argc, char const *argv[]) {
	if(argc < 5) {
		printf("Usage: %s <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossierSource> <dossierDest> \n", argv[0]);
		return -1;
	}

	//Recupération des fichiers du pair:
	DIR* rep = NULL;
	rep = opendir(argv[4]); 
	if (rep == NULL){
		perror("opendir()");
		return -1; 
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
		return -1;
	}
	//Fin de la récuperation des fichiers du pair

	printf("Creation de la socket.....");
	int sockAnnuaire = socket(AF_INET, SOCK_STREAM, 0);
	if(sockAnnuaire == -1) {
		printf("fail\n");
		perror("socket()");
		return -1;
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
		return -1;
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
		printf("Le client %d possède %d fichier(s) \n",i,tabClient[i].nbFiles );
		tabClient[i].fileList = (char**)malloc(tabClient[i].nbFiles * sizeof(char*));
		for (int j = 0; j < tabClient[i].nbFiles; j++){
			if ((res = recv(sockAnnuaire, &tailleNom, sizeof(int), 0)) < 0) {
				perror("recv tailleNom");
				return -1;
			}
			tabClient[i].fileList[j] = (char*)malloc(tailleNom * sizeof(char));
			if((res = recv(sockAnnuaire, tabClient[i].fileList[j], tailleNom, 0)) < 0) {
				perror("recv nomFichier");
				return -1;
			}
			else if(res ==0){
				return 0;
			}
			printf("\tfichier[%d] :%s\n", j,tabClient[i].fileList[j]);
		}
	}

	printf("Fermeture de la socket pair.....");
	int testClose = close(sockAnnuaire);
	if(testClose == -1) {
		printf("fail\n");
		perror("close()");
		return -1;
	}
	printf("done\n");


	pthread_t tListen1;
	pthread_t tListen2;

	// param w/ mutex part
	pthread_mutex_t tabMutex [cptFiles];
	for (int i = 0; i < cptFiles; i++) {
		pthread_mutex_init(&(tabMutex[i]), NULL);
	}
	struct paramsThreadSendFile paramsSendFile;
	paramsSendFile.nbFiles = cptFiles;
	paramsSendFile.tabMutex = tabMutex;
	for (int i = 0; i < cptFiles; i++) {
		pthread_mutex_init(&(paramsSendFile.tabMutex[i]), NULL);
	}
	//memcpy(paramsSendFile.fileList,fileList,sizeof(fileList));
	paramsSendFile.fileList = (char**)malloc(cptFiles * sizeof(char*));
	for (int i = 0; i < cptFiles; i++){
		paramsSendFile.fileList[i] = (char*)malloc(256 * sizeof(char));
		paramsSendFile.fileList[i] = fileList[i];
	}
	strcpy(paramsSendFile.rsc,argv[4]);
	// end

	struct paramsThreadServer pS;
	pS.port = atoi(argv[3]);
	pS.paramsSendFile = paramsSendFile;
	if(pthread_create(&tListen1, NULL, &serverThread, &pS)!=0){
		perror("pthread_create - tListen1");
		return -1;
	}

	struct paramsThreadClient pT;
	pT.port = htons(pS.port);
	pT.sockAddr = &addrServ;
	strcpy(pT.dest,argv[5]);
	pT.nbPair = nbClient;
	pT.tabClient = tabClient;

	if(pthread_create(&tListen2,NULL,&clientThread,&pT)){
		perror("pthread_create - tListen2");
		return -1;
	}

	/**Je ne vérifie pas que le serveur se coupe a discuter*/
	// if(pthread_join(tListen1, NULL)){
	// 	perror("pthread_join - tListen1");
	// 	return -1;
	// }
	if(pthread_join(tListen2, NULL)){
		perror("pthread_join - tListen2");
		return -1;
	}

	return 0;
}

