int mySend(int sockfd,FILE *fp, size_t len,char *nameFile,int lenNameFile){
	//For while send
	int snd =0;
	int tmp = 0;
	char ptr[1024];
	int lenBuf = 1024;
	//for while fread
	int indexFile=0;
	int rest = 0;

	if( send(sockfd,nameFile,lenNameFile,0)<0){//Envoie du nom du fichier
		perror("send() nom");
		return -1;
	}

	if(send(sockfd,&len,sizeof(int),0)<0){ //Envoie de la taille du fichier
		perror("send() taille");
		return -1;
	}
	
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
	return 0;
}

int myReceiv(int sockfd, FILE *fp) {
	int size, res;
	char buffer[1024];
	if ((res = recv(sockfd, &size, sizeof(int), 0)) < 0) {
		perror("taille_recv()");
		return -1;
	}
	int sizeFile = size;
	while (size > 0) {
		printf("%.2lf%%\r", ((double)((double)(sizeFile-size)/sizeFile))*100);
		if ((res = recv(sockfd, &buffer, 1024, 0))< 0) {
			perror("message_recv()");
			return -1;
		}
		fwrite(buffer, sizeof(buffer[0]), res, fp);

		size -= res;
	}
	return 0;
}

