#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sock_addr
#include <sys/socket.h>
#include <arpa/inet.h> //htons, inet_pton
#include <unistd.h> //close
#include <errno.h>
#include <sys/stat.h>//Taille fichier entre autre
#include <string.h>
#include <string.h>

//Stack overflow -- https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c

void str_replace(char *target, const char *needle, const char *replacement)
{
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}

int myLoopSend(int sockfd, const char *buf, size_t len, int flags){
	int tmp,rest,haveToSnd;
	haveToSnd = 0;
	rest= len;
	while(rest!=0){

		tmp= send(sockfd,&(buf[haveToSnd]),rest,0);
		if(tmp==-1){
			perror("send() ");
			return(-1);
		}
		else if (tmp==0){
			return 0;
		}
		else{
			haveToSnd+=tmp;
			rest-=tmp;
		}
	}
	return haveToSnd;
}

int mySendString(int sockfd, const char *buf, size_t len, int flags){
	int tmp = send(sockfd,&len,sizeof(int),flags);
	if(tmp ==-1){
		perror("Send taille string");
		return -1;
	}
	else if (tmp ==0){
		return 0;
	}
	else{
		printf("\nLe send a envoyé : %d octet(s)\n", tmp);
		printf("ça correspond à une taille de chaine de %lu caractères\n\n",len );
	}

	tmp = myLoopSend(sockfd,buf,len,flags);
	if(tmp ==-1){
		perror("Send string");
		return -1;
	}
	else if (tmp ==0){
		return 0;
	}
	else{
		printf("\nLe send a envoyé : %d octet(s)\n", tmp);
		printf("Equivalent à la chaine %s '\\0' compris\n\n",buf );
	}
	return tmp;

}

int mySendFile(int sockfd,FILE *fp, int len,char *nameFile,int lenNameFile){
	//For while send
	int snd =0;
	int tmp = 0;
	int lenBuf = 1025;
	char ptr[lenBuf];
	ptr[1024]='\0';

	//for while fread
	int indexFile=0;
	int rest = 0;
	int res;
	//Envoie de la taille du fichier	
	if((res=send(sockfd,&len,sizeof(int),0))<0){ 
		perror("send() taille");
		return -1;
	}else{
		printf("\nLe premier send a envoyé : %d octet(s)\n", res);
		printf("ça correspond à une taille de fichier de %d octets\n\n",len );
	}

	res =mySendString(sockfd,nameFile,lenNameFile,0);
	if(res<0){//Envoie du nom du fichier
		perror("send() nom");
		return res;
	}
	else if(res == 0){ 
		return res;
	}


	int tailleSend = 0;
	while(indexFile<len){//Envoie du contenu du fichier
		rest = fread(&ptr,sizeof(char),1024,fp);
		if(rest==-1){
			perror("fread()");
			return -1;
		}
		indexFile+=rest;
		snd =0;
		while(rest!=0){
			//print("rest : %d, send : %d\n",rest,snd);
			if(snd>0){
				printf("Buff plein");
			}
			tmp= send(sockfd,&(ptr[snd]),rest,0);
			if(tmp==-1){
				perror("send() ");
				return(-1);
			}else{
				snd+=tmp;
				rest-=tmp;
			}
			tailleSend+=tmp;
		}
		tmp=0;
	}
	printf("\n--Taille effectivement envoyé : %d--\n\n",tailleSend);
	return 0;
}

int myReceivFile(int sockfd) {
	int res,lenNameFile;
	char buffer[1024];
	char nomFichier;
	int size;
	if ((res = recv(sockfd, &size, sizeof(int), 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	else{
		printf("\nLe premier recv a reçu : %d octet(s)\n", res);
		printf("ça correspond à une taille de fichier de %d octets\n\n",size );
	}
	if ((res = recv(sockfd, &lenNameFile, sizeof(int), 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	else{
		printf("Le second recv a reçu : %d octet(s)\n", res);
		printf("ça correspond à une taille de nom fichier de %d caractères\n\n",lenNameFile );
	}
	int tailleBufferNom = lenNameFile+7;
	char filename[tailleBufferNom];
	char tmpFilename[lenNameFile];

	if ((res = recv(sockfd, &tmpFilename, lenNameFile, 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	else{
		printf("Le troisième recv a reçu : %d octet(s)\n", res);
		printf("Equivalent à la chaine %s '\\0' compris\n\n",tmpFilename );
	}
	strcpy(filename,tmpFilename);
	str_replace(filename,".","(copie).");
	printf("Le fichier %s va maintenant être créé\n",filename );

	FILE* fp = fopen(filename, "w+");
	if(fp==NULL){
		perror("fopen()");
		return -1;
	}


	int sizeFile = size;
	int tailleRcv =0;

	printf("Démarrage de la reception du fichier...\n\n");
	while (size > 0) {
		printf("Progression %.2lf%%\r", ((double)((double)(sizeFile-size)/sizeFile))*100);
		if ((res = recv(sockfd, buffer, 1025, 0))<=0) {
			perror("message_recv()");
			return -1;
		}
		tailleRcv+=res;
		fwrite(buffer, sizeof(buffer[0]), res, fp);

		size -= res;
	}
	printf("\n--Taille effectivement reçu : %d--\n\n",tailleRcv);
	fclose(fp);
	return 0;
}
