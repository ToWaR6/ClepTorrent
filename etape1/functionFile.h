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

int mySend(int sockfd,FILE *fp, size_t len,char *nameFile,int lenNameFile){
	//For while send
	int snd =0;
	int tmp = 0;
	char ptr[1024];
	int lenBuf = 1024;
	//for while fread
	int indexFile=0;
	int rest = 0;
	if(send(sockfd,&lenNameFile,sizeof(int),0)<0){//Envoie taille du nom de fichier
		perror("send() taillNom");
		return -1;
	}
	printf("Envoie taille done\n");
	if( send(sockfd,nameFile,lenNameFile,0)<0){//Envoie du nom du fichier
		perror("send() nom");
		return -1;
	}
	printf("Envoie nom done\n");
	if(send(sockfd,&len,sizeof(int),0)<0){ //Envoie de la taille du fichier
		perror("send() taille");
		return -1;
	}
	printf("Envoie taille fichier done\n");
	while(indexFile<len){//Envoie du contenu du fichier
		rest = fread(&ptr,sizeof(char),1024,fp);
		indexFile+=rest;
		if(rest==-1){
			perror("fread()");
			return -1;
		}
		snd =0;
		while(rest!=0){
			tmp= send(sockfd,&(ptr[snd]),rest,0);
			if(tmp==-1){
				perror("send() ");
				return(-1);
			}else{
				snd+=tmp;
				rest-=tmp;
			}
		}
		tmp=0;
	}
	printf("Envoie fichier done\n");
	return 0;
}

int myReceiv(int sockfd) {
	int size, res,lenNameFile;
	char buffer[1024];
	char nomFichier;
	

	if ((res = recv(sockfd, &lenNameFile, sizeof(int), 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	printf("Reception taille done%d\n",lenNameFile);
	lenNameFile +=7;
	char filename[lenNameFile];
	

	if ((res = recv(sockfd, &filename, lenNameFile, 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	str_replace(filename,".","(copie).");

	printf("Reception nom done : %s\n",filename);

	FILE* fp = fopen(filename, "w+");
	if(fp==NULL){
		perror("fopen()");
		return -1;
	}

	if ((res = recv(sockfd, &size, sizeof(int), 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	int sizeFile = size;
	while (size > 0) {
		//printf("%.2lf%%\r", ((double)((double)(sizeFile-size)/sizeFile))*100);
		if ((res = recv(sockfd, &buffer, 1024, 0))< 0) {
			perror("message_recv()");
			return -1;
		}
		printf("%d\n",res );
		fwrite(buffer, sizeof(buffer[0]), res, fp);

		size -= res;
		if(res==0){
			printf("too soon\n");
			break;
		}
	}
	printf("Bonne reception\n");
	fclose(fp);
	return 0;
}

