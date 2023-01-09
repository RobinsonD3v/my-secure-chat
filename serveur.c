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

#define NUM_THREADS	100
#define PORT 8080

typedef struct{
	int arg;
	int clientNumber;
	int *clientTotal;
}serveurStruct;

void print_log(char *str){
	char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/serveur.log")+1);
	strcpy(path,getenv("HOME"));
	strcat(path,"/my_secure_chat/serveur.log");

	FILE *logfile = fopen(path,"a");

	time_t now;time(&now);
	int h,min,s,jour,mois,annee;
	struct tm *local = localtime(&now);

	h = local->tm_hour;        
	min = local->tm_min;       
	s = local->tm_sec;       
	jour = local->tm_mday;          
	mois = local->tm_mon + 1;     
	annee = local->tm_year + 1900;  

	if (logfile)
	{
		fprintf(logfile, "[%02d/%02d/%02d %02d:%02d:%02d] %s",jour,mois,annee,h,min,s,str);
		fclose(logfile);
	}
}

void *server_thread(void *args){
	serveurStruct *serveurArgs = args;
	int newSocket = serveurArgs->arg;
	char logbuffer[100];
	sprintf(logbuffer,"serveur : new client : id -> %d\n",serveurArgs->clientNumber);
    printf("%s", logbuffer);
    print_log(logbuffer);

	while(1){
		int valread;
		msg message;
		valread = read(newSocket, &message, sizeof(message));
		
		if (valread <= 0 || message.content[0] == 0)
		{
			*serveurArgs->clientTotal = *serveurArgs->clientTotal - 1;
			sprintf(logbuffer,"serveur : client %d disconected\n",serveurArgs->clientNumber);
			print_log(logbuffer);
			//printf("client %d disconected\n",serveurArgs->clientNumber);
			break;
		}
		//printf("%d/%d | %02d/%02d/%d |%d| %s -> %s\n",serveurArgs->clientNumber,*serveurArgs->clientTotal,message.date.jour,message.date.mois,message.date.annee,message.type,message.author.pseudo, message.content);
		//printf("%d/%d %d:%d",message.date.jour,message.date.mois,message.date.heure,message.date.minute);
		parse_serv(message,&newSocket);	

	}	

	close(newSocket);
	pthread_exit(0);
}

int receive_contact(int *sock,int nb_contactes){
	char logbuff[100];
	sprintf(logbuff,"serveur : %d contacte(s) reçu :\n",nb_contactes);
	print_log(logbuff);
	printf("%d contacte(s) reçu :\n",nb_contactes);

	user *userlist = malloc(sizeof(user)* (nb_contactes+1));
	
	for(int i = 0;i < nb_contactes;i++){
		user tempo;
		read(*sock, &tempo,sizeof(tempo));
		userlist[i] = tempo;
	}
	
	for (int i = 0; i < nb_contactes; ++i)
	{
		printf("%d : .%s.\n",i,userlist[i].pseudo);
	
		if(get_user_index(userlist[i].id) == -1){
			if(add_user(userlist[i])!= 0){
				print_log("serveur : une erreur s'est produit lors de l'ajout de l'utilisateur\n\n");
			}
		}
	}

	return 0;
}

int receive_file(int *sock,unsigned long long int size,char *name){

	const char * separators = "/";
	char *previous = malloc(strlen(name));
	strcpy(previous,name);

    char *cutName = strtok ( name, separators );

    while ( cutName != NULL ) {
      	strcpy(previous,cutName);
        cutName = strtok ( NULL, separators );
    }

    char *path = malloc(strlen(strlen(getenv("HOME"))+"/my_secure_chat/dl/")+101);
    strcpy(path,getenv("HOME"));
    strcat(path,"/my_secure_chat/dl/");
    strcat(path,previous);

	FILE *fichier = fopen(path,"wb");
	char buff;

	int prev = -1;
	long int test = 0;
	printf("%s ",previous);

	for (unsigned long long int i = 0; i < size; ++i)
	{
		int avancement = (int)((float)i/(float)(size-1)*100);

		if(avancement % 10 == 0 && avancement != prev){
			if (avancement > 10)
			{
				printf("\033[3D");
			}
			else if (avancement > 0)
			{
				printf("\033[2D");
			}
			printf("%d%%", avancement);
			prev = avancement;
			fflush(stdout);
		}
		read(*sock,&buff,1);
		fwrite(&buff,1,1,fichier);
	}		
	printf("\n");
	print_log("serveur : téléchargement fichier terminer\n");
	fflush(stdout);
	fclose(fichier);
	
	return 0;
}

int parse_serv(msg message,int *sock){
	switch(message.type){

			case 0:
			{
				char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/conv/")+1);
				strcpy(path,getenv("HOME"));
				strcat(path,"/my_secure_chat/conv/");
				char *filePath = malloc(strlen(path)+strlen(message.author.id));
				strcpy(filePath,path);
				strcat(filePath,message.author.id);

				FILE* conv = fopen(filePath,"a");

				//printf("id : %s\n",message.author.id);

				/*printf("nonce : ");
				for (int i = 0; i < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; ++i)
				{
					printf("%x",message.nonce[i]);
				}
				printf("\n");

				for (int i = 0; i <= 1015; ++i)
				{
					printf("%x",message.content[i]);
				}
				printf("\n");
				*/
				if (conv)
				{
					fwrite(&message,sizeof(message),1,conv);
					fclose(conv);
				}
				else{
					print_log("erreur : historique inaccessible !!!\n");
					free(filePath);
					return -1;
				}

				free(filePath);
				break;
			}
			case 1:
			{
				int nb_contactes = atoi(message.content);
				receive_contact(sock,nb_contactes);
				break;
			}
			case 2:{

				if (message.content[0] == '?')
				{	
					char buff = 'n';
					extend_user me = get_you();

					if (me.user.online == 'y')
						buff = 'y';

					char logbuff[100];
					sprintf(logbuff,"serveur : send presence : %c\n",buff);
					print_log(logbuff);

					send(*sock,&buff,sizeof(buff),0);
				}
				break;
			}
			case 3:{
				unsigned long long int size = 0;
				char name[100];

				sscanf(message.content,"%llu:%s",&size,name);

				char logbuff[100];
				sprintf(logbuff,"serveur : fichier : %lluo\n",size);
				print_log(logbuff);

				printf("[%02d/%02d %02d:%02d] : reception de ",message.date.jour,message.date.mois,message.date.heure,message.date.minute);
				fflush(stdout);
				
				receive_file(sock,size,name);
				return 0;
				break;
			}

			case 4:{
				fflush(stdout);
				//printf("new p_key : \n");
				unsigned char alice_pkey[crypto_box_PUBLICKEYBYTES+1];
				for (int i = 0; i <= crypto_box_PUBLICKEYBYTES; ++i)
				{
					//printf("%x",message.content[i]);
					alice_pkey[i] = message.content[i];
				}
				//printf("\n\n");
				fflush(stdout);

				unsigned char bob_publickey[crypto_box_PUBLICKEYBYTES];
				unsigned char bob_secretkey[crypto_box_SECRETKEYBYTES];
				crypto_box_keypair(bob_publickey, bob_secretkey);

				unsigned char nonce[crypto_box_NONCEBYTES];
				unsigned char s_key[crypto_aead_chacha20poly1305_KEYBYTES];
				unsigned char ciphertext[crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES];
				unsigned long long ciphertext_len = crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES;
				
				crypto_aead_chacha20poly1305_keygen(s_key);
				randombytes_buf(nonce, sizeof nonce);

				/*printf("\n\nsecret key : ");
				for (int i = 0; i <= crypto_aead_chacha20poly1305_KEYBYTES; ++i)
				{
					printf("%x",s_key[i]);
				}
				*/

				if (crypto_box_easy(ciphertext, s_key, crypto_aead_chacha20poly1305_KEYBYTES+2, nonce,
                    alice_pkey, bob_secretkey) != 0) {
				    print_log("erreur : lors du chiffrement de la clée privée\n");				
				}


				struct struct_key
				{
					char nonce[crypto_box_NONCEBYTES+1];
					char bob_key[crypto_box_PUBLICKEYBYTES+1];
					char chiffre_s_key[crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES];
				}struct_key;

				//printf("\n\n\nnonce : ");
				for (int i = 0; i <= crypto_box_NONCEBYTES; ++i)
				{
					struct_key.nonce[i] = nonce[i];
					//printf("%x",struct_key.nonce[i]);
				}
				//printf("\n\nbob_key : ");
				for (int i = 0; i <= crypto_box_PUBLICKEYBYTES; ++i)
				{
					struct_key.bob_key[i] = bob_publickey[i];
					//printf("%x",struct_key.bob_key[i]);
				}
				//printf("\n\ns_key : ");
				for (int i = 0; i <= crypto_aead_chacha20poly1305_KEYBYTES + 1 + crypto_box_MACBYTES; ++i)
				{
					struct_key.chiffre_s_key[i] = ciphertext[i];
					//printf("%x",struct_key.chiffre_s_key[i]);
				}

				send(*sock,&struct_key,sizeof(struct_key),0);
				//printf("\n----------------\n\n");
				fflush(stdout);

				char *path = malloc(strlen(getenv("HOME")) + strlen("/my_secure_chat/keys/")+1);
				strcpy(path,getenv("HOME"));
				strcat(path,"/my_secure_chat/keys/");

				char *filePathKey = malloc(strlen(path)+strlen(message.author.id));
				strcpy(filePathKey,path);
				strcat(filePathKey,message.author.id);
				FILE *keyFile = fopen(filePathKey,"wb");

				if (!keyFile){
					print_log("erreur : création fichier key\n");
					return -1;
				}

				fwrite(&s_key,sizeof(s_key),1,keyFile);
				fclose(keyFile);

				break;
			}
			default:
				break;
		}
}

void *serveur_process(void *args)
{	
	print_log("serveur : start\n");
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
	    		print_log("serveur : thread failled\n");
	    	}
	   		*clientNumber = *clientNumber + 1;

	    }
	}
    
    shutdown(serverSocket, SHUT_RDWR);
    free(clientNumber);
    return 0;
}
