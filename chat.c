#include "chat.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <uuid/uuid.h>
#include <sys/stat.h>
//#include <libexplain/fwrite.h>


#define NUM_THREADS	4
//#define LEN_ID 8

#define PORT 8080

typedef struct{
	int arg;
	int clientNumber;
	int *clientTotal;
}serveurStruct;


/*int serveur()
{	
	pthread_t threads[NUM_THREADS];
    int serverSocket, newSocket;
    int *clientNumber = malloc(sizeof(int));*clientNumber=0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[sizeof(msg)] = { 0 };
    char* hello = "Hello from server";
 	
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    if (setsockopt(serverSocket, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    if (bind(serverSocket, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while(*clientNumber < 10){
	    if (listen(serverSocket, 3) < 0) {
	        perror("listen");
	        exit(EXIT_FAILURE);
	    }
	    else{
	    	if ((newSocket = accept(serverSocket,
	    	                        (struct sockaddr*)&address,
	    	                        (socklen_t*)&addrlen)) < 0) {
	        perror("accept");
	        exit(EXIT_FAILURE);
	    	}
	    	serveurStruct *serveurArgs = malloc(sizeof(serveurStruct));
	    	
	    	serveurArgs->arg = newSocket;
	    	serveurArgs->clientNumber = *clientNumber;
	    	serveurArgs->clientTotal = clientNumber;
	    	

	    	if(pthread_create(&threads [*clientNumber],NULL,server_thread,serveurArgs) != 0){
	    		printf("thread failled\n");
	    	}
	   		*clientNumber = *clientNumber + 1;

	    }
	}
    
    shutdown(serverSocket, SHUT_RDWR);
    free(clientNumber);

    return 0;
}
 */
int client()
{
	clear();
	extend_user me = get_you();
	if (me.pass[0] == 0)
	{
		user you;
		
		char *buffer = malloc(37);
		printf("ton pseudo : ");
		fgets(buffer,32,stdin);
		buffer[strlen(buffer)-1] = '\0';
		strcpy(you.pseudo,buffer);
		memset(buffer,0,37);

		printf("ton ip : ");
		fgets(buffer,15,stdin);
		buffer[strlen(buffer)-1] = '\0';
		strcpy(you.ip,buffer);
		memset(buffer,0,37);

		uuid_t binuuid;
	    uuid_generate_random(binuuid);
	    uuid_unparse(binuuid, buffer);
	    strcpy(you.id,buffer);
	    printf("ton id : %s\n",you.id);
	    memset(buffer,0,37);

	    printf("veux tu que ton profil soit partager automatiquement ? [Y/n] ");
	    fgets(buffer,3,stdin);
	    if (buffer[0] != 'n')
	    {
	    	buffer[0] = 'y';
	    }
	    you.share = buffer[0];
	    memset(buffer,0,37);

	    printf("et pour finir ton mot de passe (max 30 caractères) : ");
	    fgets(buffer,31,stdin);
	    
	    make_you(you,buffer);
	    free(buffer);

	    me = get_you();
	}


	printf("bienvenue %s:%s !\n\n",me.user.pseudo,me.user.id);
	
	user recipient;
	printf("qui tu veux contacter : ");
	fgets(recipient.pseudo,32,stdin);
	recipient.pseudo[strlen(recipient.pseudo)-1] = '\0';

	recipient = get_user(recipient.pseudo);

	if (recipient.id[0] == 0)
	{
		printf("pas d'utilisateur trouver\n");
		exit(1);
	}

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    if (inet_pton(AF_INET, recipient.ip, &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    msg hello;
    hello.type = 1;
    strcpy(hello.dest,recipient.id);
	hello.author = me.user;
	hello.date = get_date();
	strcpy(hello.content,"hello from client !");
    send(sock, &hello, sizeof(hello), 0);
    printf("Hello message sent s:%s\n",hello.content);

    pthread_t thread_hist;
    struct hist_args
    {
    	char id[37];
    	long int nbr_msg;
    }hist_args;

    strcpy(hist_args.id,recipient.id);

    char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/conv/")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/conv/");

	char *filePath = malloc(strlen(path)+strlen(recipient.id)+1);
	strcpy(filePath,path);
	strcat(filePath,recipient.id);

    FILE *testFichier = fopen(filePath,"a");
	    	if (!testFichier)
	    	{
	    		exit(1);
	    	}
	fclose(testFichier);
    pthread_create(&thread_hist, NULL, afficher_historique, &hist_args);

    while(1){
    	msg message;

 		message.type = 0;
 		strcpy(message.dest,recipient.id);
 		message.author = me.user;
 		message.date = get_date();

    	fgets(message.content,1000,stdin);
    	message.content[strlen(message.content)-1] = '\0';
    	
    	send(sock, &message, sizeof(msg), 0);
    	if (strcmp(message.content,"exit") == 0 || message.content[0] == 0)
   		{
   			break;
   		}
   		else{

			FILE *fichier = fopen(filePath,"a");

	    	if (!fichier)
	    	{
	    		printf("no file found\n");
	    		exit(1);
	    	}
	    	
	    	if(ftrylockfile(fichier) != 0){
	    		printf("fichier pas locker\n");
	    	}
	    	fwrite(&message,sizeof(message),1,fichier);
	    	
	    	fclose(fichier);
   		}
   		free(filePath);
    }
    
    close(client_fd);
    return 0;
}

void *afficher_historique(void *args){
	
	struct args_hist{
		char id[37];
    	long int nbr_msg;	
	}*args_hist;
	
	args_hist = args;
	char id[37];
	strcpy(id,args_hist->id);

	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/conv/")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/conv/");
	char *filePath = malloc(strlen(path)+strlen(id));
	strcpy(filePath,path);
	strcat(filePath,id);

	unsigned long previous = 0;
	int first = 0;
	while(1){
		
		struct stat sb;
	    if (stat(filePath, &sb) == -1) {
	        perror("stat");
	        return NULL;
	    }
	   	
	    if (previous != sb.st_size || first == 0)
	    {
	    	clear();	
    		
	    	FILE *fichier = fopen(filePath,"r");
	    	if (!fichier)
	    	{
	    		printf("no file found\n");
	    		return NULL;
	    	}
	    	
	    	msg message;
	    	while(fread(&message,sizeof(message),1,fichier)){
	    		char *decrypted = malloc(1000);
	    		unsigned long long decrypted_len;

	    		char *path2 = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/keys/")+1);
			    strcpy(path2,getenv("HOME"));
			    strcat(path2,"/my_secure_chat/keys/");
				char *filePathKey = malloc(strlen(path2)+strlen(id));
				strcpy(filePathKey,path2);
				strcat(filePathKey,id);
				FILE *keyFile = fopen(filePathKey,"rb");

				if (!keyFile){
					break;
				}
				
				unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
				fread(&key,sizeof(key),1,keyFile);
				fclose(keyFile);
				
				/*printf("nonce : ");
				for (int i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; ++i)
				{
					printf("%x",message.nonce[i]);
				}
				printf("\n");
				printf("l : %ld\n",strlen(message.content) );*/
				
	    		if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted, 
	    								   &decrypted_len,
                                           NULL,
                                           message.content, 1015,
                                           ADDITIONAL_DATA,
                                           ADDITIONAL_DATA_LEN,
                                           message.nonce, key) != 0) {
			    printf("une erreur a eu lieu\n");
			}

	    		printf("%s : %s\n\n",message.author.pseudo,decrypted);
	    	}
	    	printf("-> ");
	    	fflush(stdout);	
	    	
	    	fclose(fichier);	    	
	    	previous = sb.st_size;
	    }
	    first++;
	    usleep(150000);
	}
	free(filePath);
}

temps get_date(){
	time_t now;
	time(&now);
	temps date;
	struct tm *local = localtime(&now);
	date.jour = local->tm_mday;
	date.mois = local->tm_mon + 1;
	date.annee = local->tm_year + 1900;
	return date;
}

user get_user(char *pseudo){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");

	FILE *userList = fopen(path,"r");
	user utilisateur;
    
    memset(&utilisateur,0,sizeof(utilisateur));
		
	user *previous = malloc(sizeof(user));		

	if (!userList)
	{
		return utilisateur;
	}
	int trouver = 0;
	while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
		if(strcmp(pseudo,utilisateur.pseudo) == 0 && utilisateur.del == 'n'){
			trouver++;			
			if (trouver < 2)
			{
				*previous = utilisateur;
			}
			else {
				if(trouver == 2){
					printf("1\tpseudo : %s\n\tid : %s\n\n",previous->pseudo,previous->id);
				}
				printf("%d\tpseudo : %s\n\tid : %s\n\n",trouver,utilisateur.pseudo,utilisateur.id);
			}
			
		}
	}

	fseek(userList, 0, SEEK_SET);

	if (trouver > 1)
	{
		int selectione;
		printf("\nplusieur personnes avec le pseudo %s on été trouver\n",pseudo);
		printf("inqique le numéro de celui que tu veut contacter : ");
		scanf("%d",&selectione);
		getchar();

		trouver = 0;

		while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
			if(strcmp(pseudo,utilisateur.pseudo) == 0 && utilisateur.del == 'n'){
				trouver++;
			}
			if (trouver == selectione)
			{
				break;
			}
		}
	}
	else if(trouver == 1){
		while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
			if(strcmp(pseudo,utilisateur.pseudo) == 0){
				break;
		}
		}
	}
	else{
		memset(&utilisateur,0,sizeof(utilisateur));
	}
	
	fclose(userList);
	free(previous);
	return utilisateur;
}

int add_user(user utilisateur){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *userList = fopen(path,"a");

	if (!userList)
	{
		return(-1);
	}
	fwrite(&utilisateur,sizeof(utilisateur),1,userList);
	fclose(userList);
	return(0);
}

int del_user(char *id){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *userList = fopen(path,"r+");

	int index = 0;
	if (!userList)
	{
		return -1;
	}

	user utilisateur;
	 char find = 'n';
	while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
		if(strcmp(id,utilisateur.id) == 0){
			find = 'y';
			break;		
		}
		index++;
	}
	if (find == 'n')
		return 1;

	utilisateur.del = 'y';
	fseek(userList, index*sizeof(user), SEEK_SET);
	fwrite(&utilisateur, sizeof(utilisateur), 1, userList);

	fclose(userList);
	return 0;
}
int get_user_index(char *id){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *userList = fopen(path,"r");

	user utilisateur;			

	if (!userList)
	{
		return -2;
	}
	int index = 0;int found = 0;
	while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
		if(strcmp(id,utilisateur.id) == 0){
			found = 1;
			break;
		}
		index++;
	}

	fclose(userList);
	if (found == 0)
	{
		return -1;
	}
	
	return index;
}
user get_user_by_index(int index){
	user utilisateur;

	memset(&utilisateur,0,sizeof(utilisateur));

	struct stat sb;
    if (stat("userlist", &sb) == -1) {
        perror("stat");
        return utilisateur;
    }
    int total = sb.st_size / sizeof(user);

    if (index > total || index < 0)
    {
    	return utilisateur;
    }
    
    char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *userlist = fopen(path,"r");
    
    if (!userlist)
    {
    	return utilisateur;
    }

	fseek(userlist, index*sizeof(user), SEEK_SET);
	fread(&utilisateur,sizeof(user),1,userlist);

	return utilisateur;
}
user get_user_by_id(char *id){

	return get_user_by_index(get_user_index(id));
}
int clean_userlist(void){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *old = fopen(path,"r");

	char *path2 = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist_temp")+1);
    strcpy(path2,getenv("HOME"));
    strcat(path2,"/my_secure_chat/userlist_temp");

	FILE *new = fopen(path2,"w+");

	if (!old || !new)
	{
		return -1;
	}
	user utilisateur;
	int index = 0;
	while(fread(&utilisateur,sizeof(utilisateur),1,old)){
		if (utilisateur.del != 'y')
		{
			fwrite(&utilisateur,sizeof(utilisateur),1,new);
		}
		else{
			index++;
		}
	}
	remove("userlist");rename("userlist_temp","userlist");
	printf("\n%d utilisateur(s) retirer avec succès\n\n", index);
	fclose(old);fclose(new);
	return 0;
}

int display_userlist(char all){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *userList = fopen(path,"r");

	user utilisateur;
	if (!userList)
	{
		return -1;
	}
	int trouver = 0;
	printf("\n");
	while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
			if(utilisateur.del == 'y' && all == 'n' )
				;
			else{
				printf("pseudo : %s\nid : %s\nip :%s\n\n",utilisateur.pseudo,utilisateur.id,utilisateur.ip);
			}	
				

	}
	fclose(userList);
	return 0;
}

int modify_user(char *id,user new){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/userlist")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/userlist");
    
	FILE *old = fopen(path,"r+");
	
	if (!old)
	{
		return -1;
	}

	int index = get_user_index(id);
	if (index < 0)
	{
		return 1;
	}
	if(strcmp(id,new.id) != 0){
		return 2;
	}
	if (new.id[0])
	{
		fseek(old, index*sizeof(new), SEEK_SET);
		fwrite(&new, sizeof(new), 1, old);
	}
	else{
		return -1;
	}
	fclose(old);
	return 0;
}
int modify_you(extend_user you){
	char *path = malloc(strlen(getenv("HOME"))+strlen("/my_secure_chat/you")+1);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/you");
    
	FILE *old = fopen(path,"r+");

	if (!old)
	{
		return -1;
	}
	extend_user old_you = get_you();
	if (strcmp(you.user.id,old_you.user.id) == 0)
	{
		if(make_you(you.user,you.pass) != 0){
			return -1;
		}
		return 0;
	}
	else{
		return 1;
	}	
}
int make_you(user you,char pass[257]){

	char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/you")+1);
	strcpy(path,getenv("HOME"));
	strcat(path,"/my_secure_chat/you");

	FILE *youFile = fopen(path,"w");

	if (!youFile)
	{		
		printf("erreur ouverture make_you\n");
		return -1;
	}

	extend_user ext_you;

	ext_you.user = you;
	strcpy(ext_you.pass,pass); 

	fwrite(&ext_you,sizeof(ext_you),1,youFile);
	fclose(youFile);

	return 0;
}

extend_user get_you(){
	char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/you")+1);
	strcpy(path,getenv("HOME"));
	strcat(path,"/my_secure_chat/you");

	FILE *you = fopen(path,"r");
	extend_user ext_you;
	
	if (!you)
	{	
		ext_you.pass[0] = 0;
		return ext_you;
	}

	fread(&ext_you,sizeof(ext_you),1,you);
	fclose(you);
	return(ext_you);
}

int display_online(void){
	char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/userlist")+1);
	strcpy(path,getenv("HOME"));
	strcat(path,"/my_secure_chat/userlist");

	FILE * userList = fopen(path,"r");
	user utilisateur;
	if (!userList)
	{
		return -1;
	}
	int trouver = 0;
	printf("\n");
	int nbr = 0;
	int total = 0;
	while(fread(&utilisateur,sizeof(utilisateur),1,userList)){
			if(utilisateur.del != 'y' && utilisateur.online == 'y' ){
				printf("pseudo : %s\nid : %s\nip :%s\n\n",utilisateur.pseudo,utilisateur.id,utilisateur.ip);
				nbr++;
			}
			total++;
	}
	printf("%d/%d personne%c en ligne\n\n",nbr,total,(nbr < 2)? ' '  : 's');
	fclose(userList);
	return 0;
}

int total_user(){
	struct stat sb;
    if (stat("userlist", &sb) == -1) {
        perror("stat");
        return -1;
    }

    return sb.st_size / sizeof(user);
}

int get_presence(void){

	extend_user me  = get_you();
	int totalUser = total_user();
	int online = 0;
	for (int i = 0; i < totalUser; ++i)
	{		
		int sockfd, connfd;
	    struct sockaddr_in servaddr, cli;
	   	
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfd == -1) {
	        printf("socket creation failed...\n");
	        exit(0);
	    }
	    
	    bzero(&servaddr, sizeof(servaddr));
	   	
	   	user current = get_user_by_index(i);

	    servaddr.sin_family = AF_INET;
	    servaddr.sin_addr.s_addr = inet_addr(current.ip);
	    servaddr.sin_port = htons(PORT);
	   
	    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
	        current.online = 'n';
	    }
	    else
	    {
	    	msg message;
			message.type = 2;
			message.author = me.user;
			strcpy(message.dest,current.id);
			message.date = get_date();
			sprintf(message.content,"%c",'?');

			send(sockfd,&message,sizeof(message),0);

			char buff;
			read(sockfd,&buff,sizeof(buff));

			if (buff == 'y')
			{
				current.online = 'y';
				online++;
			}
			else{
				current.online = 'n';
			}
	    }

	    modify_user(current.id,current);

	    close(sockfd);
	}
	printf("\n%d/%d personne%c en ligne, fait \"online\" pour les afficher\n\n",online,totalUser,(online < 2)? ' ' : 's');
	return 0;
}


int generate_private_key(char *path){
	return 0;
}
