int mySend(int sockfd,FILE *fp, size_t len){
	//For while send
	int snd =0;
	int tmp = 0;
	char ptr[1024];
	int lenBuf = 1024;
	//for while fread
	int indexFile=0;
	printf("%d\n",len );
	int rest = 0;
	if(send(sockfd,&len,sizeof(int),0)<0){
		perror("send taille");
		return(-1);
	}
	while(indexFile<len){
		rest = fread(&ptr,sizeof(char),1024,fp);
		indexFile+=rest;
		printf("rest : %d\n", rest);
		//scanf("%1s");
		if(rest==-1){
			perror("fread()");
			return -1;
		}
		//printf("%s\n",ptr );
printf("%d / %d",indexFile,len );
		snd =0;
		while(rest!=0){

			tmp= send(sockfd,&ptr[snd],rest,0);
			printf("%d\n",tmp );
			if(tmp==-1){
				perror("send() ");
				return(-1);
			}else{
				snd+=tmp;
				rest-=tmp;
			}
		}

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
	printf("res :%d | reçu : %d\n", res,size);
	while (size > 0) {
		printf("Il reste %d octet(s)\n", size);
		if ((res = recv(sockfd, buffer, size, 0))< 0) {
			perror("message_recv()");
			return -1;
		}

		fwrite(buffer, sizeof(buffer[0]), res, fp);

		size -= res;
	}

	return 0;
}

