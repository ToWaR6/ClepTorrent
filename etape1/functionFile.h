int mySendFile(int sockfd,FILE *fp, size_t len){
	int snd =0;
	int tmp = 0;
	int rest= len;
	char ptr[1024];
	int indexPtr = 0;
	char actualChar;
	if(send(sockfd,&len,sizeof(int),0)<0){
		perror("send taille");
		return(-1);
	}
	do{
		actualChar = fgetc(fp); // On lit le caractère
		ptr[indexPtr]=actualChar; // On le stock
		indexPtr++;
    } while (actualChar != EOF && indexPtr<=1024);
    indexPtr=0;
   
	while(snd<len){
		tmp= send(sockfd,&ptr[snd],rest,0);
		if(snd==-1){
			perror("send() ");
			return(-1);
		}else{
			snd+=tmp;
			rest-=tmp;
			do{
				actualChar = fgetc(fp); // On lit le caractère
				ptr[indexPtr]=actualChar; // On le stock
				indexPtr++;
			} while (actualChar != EOF && indexPtr<=1024);
			indexPtr=0;
		}
	}
	return 0;
}

int myReceiv(int sockfd, FILE *fp) {
	int size, res;
	char buffer[1024];
	if (res = recv(sockfd, &size, sizeof(int), 0) < 0) {
		perror("taille_recv()");
		return -1;
	}

	while (size > 0) {
		// printf("Il reste %d octet(s)\n", size);
		if (res = recv(sockfd, &buffer, size, 0) < 0) {
			perror("message_recv()");
			return -1;
		}

		fwrite(buffer, sizeof(buffer[0]), res, fp);

		size -= res;
	}

	return 0;
}

