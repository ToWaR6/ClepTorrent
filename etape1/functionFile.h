int mySendFile(int sockfd,FILE *fp,size_t len){
	int snd =0;
	int tmp = 0;
	int rest= len;
	char ptr[1024];
	int indexPtr = 0;
	char actualChar;
	if(send(sockfd,&len,sizeof(int),0)<0){
		perror("send taille");
		exit(-1);
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

