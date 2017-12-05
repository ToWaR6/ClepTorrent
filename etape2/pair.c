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
	char** fileList;
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



int main(int argc, char const *argv[]) {
	if(argc < 4) {
		printf("Usage: %s <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossierSource> \n", argv[0]);
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

	//Création de ma socket

	int mySock = socket(AF_INET,SOCK_STREAM,0);
	if (mySock < 0) {
		perror("socket()");
		return -1;
	}
	printf("Socket créée\n");
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = (short) htons(atoi(argv[3]));

	int err = bind(mySock, (struct sockaddr*)&ad, sizeof(ad));
	if (err < 0) {
		perror("bind()");
		close(mySock);
		return -1;
	}
	int res;
	//Envoie du fichier
	if((res = send (sockAnnuaire,&ad.sin_port,sizeof(short),0))<0){
		perror("send() port");
		return -1;
	}
	printf("Port %d envoyé \n",ad.sin_port);

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
		tailleNom = strlen(fileList[i])+1;
		//Envoie du nombre de caractère
		if((res=send(sockAnnuaire,&tailleNom,sizeof(int),0))<0){ 
			perror("send() taille");
			return -1;
		}

		//Envoie nom du fichier
		if ((res=send(sockAnnuaire,&fileList[i],sizeof(int),0))<0){
			perror("send() nomFichier");
			return -1;
		}
	}



	printf("Fermeture de la socket client.....");
	int testClose = close(sockAnnuaire);
	if(testClose == -1) {
		printf("fail\n");
		perror("close()");
		exit(EXIT_FAILURE);
	}
	printf("done\n");
	
	return 0;
}