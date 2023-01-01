#include "chat.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <uuid/uuid.h>
#include <sys/stat.h>
#include <errno.h>

//#include <libexplain/fwrite.h>

/* j'ai un peu de taf ^^*/
// va falloir faire un serveur plus propre et ça vas être pas mal...
int send_special_message(int type,long int size,int *sockfd){
	
	msg message;
	message.type = type;
	sprintf(message.content,"%ld",size);

	send(*sockfd,&message,sizeof(message),0);

	return 0;
}

int send_msg(void *argp){
	struct arg
	{
		int *sock;
		msg message;
	}*args;

	args = argp;

	send(*(args->sock), &(args->message), sizeof(args->message), 0);

	return 0;
}
//vérifier le share puis envoyé la list
//en penssant à faire le parsing serveur 

int send_contactes(void *argp){
	struct arg
	{
		int *sock;
		int nbr;
		char id[LEN_ID];
		extend_user author;
	}*args;

	args = argp;
	srand(time(NULL));
	int total = total_user();
	if (args->nbr > total)
		args->nbr = total;

	user *userlist = malloc(sizeof(user)* (args->nbr)+1);
	int *indexlist = malloc(sizeof(int) * (args->nbr+1));
	int trueTotal = 0;
	for (int i = 0; i < args->nbr; ++i)
	{	
		int randNum; char diff = 'y';
		while(1){
			randNum = rand() % total;
			indexlist[i] = randNum;

			for (int j = 0; j < i; ++j)
			 {
			 	if(indexlist[j] == randNum){
			 		diff = 'n';
			 		break;
			 	}
			 	else if(j == i-1)
			 		diff = 'y';		 	 	
			 }
			 if (diff == 'y')
			 	break;		 
		}
		user tempo = get_user_by_index(randNum);
		if (tempo.share == 'y' && tempo.del == 'n')
		{
			userlist[trueTotal] = tempo;
			trueTotal++;
		}
	}

	if (trueTotal > 0)
	{
		msg firstMsg;
		firstMsg.type = 1;
		firstMsg.author = args->author.user;
		strcpy(firstMsg.dest,args->id);
		firstMsg.date = get_date();
		sprintf(firstMsg.content,"%d",trueTotal);
		
		userlist = (user *)realloc(userlist,(trueTotal+1)*sizeof(user));
		if (!userlist)
		{
			return -1;
		}

		send(*(args->sock), &firstMsg, sizeof(firstMsg), 0);
		for(int j=0;j<trueTotal;j++){
			user tempo2 = userlist[j];
			send(*(args->sock), &tempo2, sizeof(tempo2),0);
		}
		
		printf("%d utilisateur%c partagé\n",trueTotal, (trueTotal==1) ? ' ' : 's');
	}
	else{
		printf("aucun utilisateur envoyé");
	}
}

int send_file(void *argp){

	struct arg
	{
		int *sock;
		char *path;
		char id[LEN_ID];
		extend_user author;
	}*args;

	args = argp;
	FILE *fichier = fopen(args->path,"rb");
	if (!fichier)
	{
		return -1;
	}

	fseek(fichier, 0, SEEK_END);
	long long int size = ftell(fichier);
	fseek(fichier, 0, SEEK_SET);
	if (size <= 0)
	{
		return -1;
	}
	msg firstMsg;
	firstMsg.type = 3;
	firstMsg.author = args->author.user;
	strcpy(firstMsg.dest,args->id);
	firstMsg.date = get_date();
	sprintf(firstMsg.content,"%lld",size);
	printf("s : %ld\n",send(*(args->sock),&firstMsg,sizeof(firstMsg),0));

	char buff;
	
	for (long long int i = 0; i < size; ++i)
	{
		fread(&buff,1,1,fichier);
		send(*(args->sock),&buff,1,0);
	}
	

	fclose(fichier);
	return 0;
}

int send_new_pub_key(void *argp){
	struct arg
	{
		int *sock;
		msg message;
		unsigned char alice_secretkey[crypto_box_SECRETKEYBYTES];
	}*args;

	args = argp;

	send(*(args->sock), &(args->message), sizeof(args->message), 0);
	printf("\npkey send\n\n");

	struct struct_key
	{
		char nonce[crypto_box_NONCEBYTES+1];
		char bob_key[crypto_box_PUBLICKEYBYTES+1];
		char chiffre_s_key[crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES];
	}struct_key;

	long int ciphertext_len = crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES;
	printf("en attente réponse serv\n\n");

	read(*(args->sock), &struct_key, sizeof(struct_key));
	printf("réponse serv reçu\n\n");

	printf("nonce : ");

	for (int i = 0; i <= crypto_box_NONCEBYTES; ++i)
	{
		printf("%x",struct_key.nonce[i]);
	}
	printf("\n\nbob_key : ");
	for (int i = 0; i <= crypto_box_PUBLICKEYBYTES; ++i)
	{
		printf("%x",struct_key.bob_key[i]);
	}
	printf("\n\nc_s_key : ");
	for (int i = 0; i <= crypto_aead_chacha20poly1305_KEYBYTES + 2 + crypto_box_MACBYTES; ++i)
	{
		printf("%x",struct_key.chiffre_s_key[i]);
	}

	unsigned char decrypted[crypto_aead_chacha20poly1305_KEYBYTES+2];

	if (crypto_box_open_easy(decrypted, struct_key.chiffre_s_key, ciphertext_len, struct_key.nonce,
	                         struct_key.bob_key, args->alice_secretkey) != 0) {
	    printf("erreur lors du déchifremment s_key\n");
	}

	printf("\n\n\nsecret_key : ");

	for (int i = 0; i < crypto_aead_chacha20poly1305_KEYBYTES+1; ++i)
	{
		printf("%x",decrypted[i]);
	}

	char *path = "keys/";

	char *filePathKey = malloc(strlen(path)+strlen(args->message.dest));
	strcpy(filePathKey,path);
	strcat(filePathKey,args->message.dest);
	FILE *keyFile = fopen(filePathKey,"wb");

	if (!keyFile){
		printf("erreur création fichier key");
		return -1;
	}

	fwrite(&decrypted,sizeof(decrypted),1,keyFile);
	printf("\ns : %ld\n\n",sizeof(decrypted));
	fclose(keyFile);

	return 0;
}

int client2(user recipient, int (*send_function)(void *),void *(*history_function)(void *args),int (*parse_function)(char *,int,user *)) {
	extend_user me  = get_you();
	
	int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    
    bzero(&servaddr, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(recipient.ip);
    servaddr.sin_port = htons(PORT);
   
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connecté à %s\n",recipient.pseudo);

    
    	
    char *path = "conv/";
			char *filePath = malloc(strlen(path)+strlen(recipient.id)+1);
			strcpy(filePath,path);
			strcat(filePath,recipient.id);

    FILE *testFichier = fopen(filePath,"a");

	    	if (!testFichier)
	    	{
	    		testFichier = fopen(filePath,"w");
	    		if (!testFichier)
		    	{
		    		return -1;
		    	}
	    	}

	fclose(testFichier);

	pthread_t thread_hist;
	if (history_function)
    {
	    struct hist_args
	    {
	    	char id[37];
	    	long int nbr_msg;
	    }hist_args;
		strcpy(hist_args.id,recipient.id);

		pthread_create(&thread_hist, NULL, (*history_function), &hist_args);
	}
	if (parse_function)
	{
		while(1){
			
			char *buffer = malloc(1000);
			
			fgets(buffer,1000,stdin);
			buffer[strlen(buffer)-1] = '\0';

			switch((*parse_function)(buffer,1,&recipient)){
				case 1 :
					free(buffer);
					close(sockfd);
					
					return 0;
				case 2 :{
					msg message;
					message.type = 0;
					strcpy(message.dest,recipient.id);
					message.author = me.user;
					message.date = get_date();

					unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
					unsigned char crypted[CRYPT_MESSAGE_LEN + crypto_aead_xchacha20poly1305_ietf_ABYTES];
					unsigned long long crypted_len;

					char *path = "keys/";
					char *filePathKey = malloc(strlen(path)+strlen(recipient.id));
					strcpy(filePathKey,path);
					strcat(filePathKey,recipient.id);
					FILE *keyFile = fopen(filePathKey,"rb");

					if (!keyFile){
						printf("erreur ouverture fichier keys");
						return -1;
					}
					
					unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
					fread(&key,sizeof(key),1,keyFile);
					fclose(keyFile);

					randombytes_buf(nonce, sizeof nonce);

					crypto_aead_xchacha20poly1305_ietf_encrypt(crypted, &crypted_len,
					                                           buffer, CRYPT_MESSAGE_LEN,
					                                           ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
					                                           NULL, nonce, key);
					for (int i = 0; i <= crypted_len; ++i)
					{
						message.content[i] = crypted[i];
					}
					for (int i = 0; i <= sizeof(nonce); ++i)
					{
						message.nonce[i] = nonce[i];
					}
					
					message.securplus = 1;

					struct arg
					{
						int *sock;
						msg message;
					}args;

					args.sock = &sockfd;
					args.message = message;

					if (send_function)
					{
						(*send_function)(&args);
					}
					else
						return -1;

					FILE *fichier = fopen(filePath,"a");

			    	if (!fichier)
			    	{
			    		printf("no file found\n");
			    		return -1;
			    	}

			    	fwrite(&message,sizeof(message),1,fichier);
			    	fclose(fichier);
					break;
					}
				case 3:{
					struct arg
					{
						int *sock;
						char *path;
						char id[LEN_ID];
						extend_user author;
					}args;

					char *filename = malloc(100);
					printf("path to file : ");
					fgets(filename,100,stdin);
					
					filename[strlen(filename)-1] = '\0';

					args.sock = &sockfd;
					args.author = me;
					strcpy(args.id,recipient.id);

					args.path = malloc(strlen(filename+1));
					strcpy(args.path,filename);					

					if(send_file(&args) == 0){
						printf("fichier envoyé avec succès !\n\n");
					}
					else{
						printf("une erreur c'est produite\n\n");
					}
					break;
				}	
				case 4:{
					struct arg
					{
						int *sock;
						int nbr;
						char id[LEN_ID];
						extend_user author;
					}args;

					args.nbr = 3;
					args.sock = &sockfd;
					args.author = me;
					strcpy(args.id,recipient.id);

					send_contactes(&args);
					break;
				}
			case 5:{
				msg message;
				message.type = 4;
				strcpy(message.dest,recipient.id);
				message.author = me.user;
				message.date = get_date();

				unsigned char alice_publickey[crypto_box_PUBLICKEYBYTES];
				unsigned char alice_secretkey[crypto_box_SECRETKEYBYTES];
				crypto_box_keypair(alice_publickey, alice_secretkey);
				
				struct arg
					{
						int *sock;
						msg message;
						unsigned char alice_secretkey[crypto_box_SECRETKEYBYTES+1];

					}args;

				for (int i = 0; i <= crypto_box_PUBLICKEYBYTES; ++i)
				{
					message.content[i] = alice_publickey[i];
					args.alice_secretkey[i] = alice_secretkey[i];
					printf("%x",message.content[i]);
				}


				args.sock = &sockfd;
				args.message = message;

				send_new_pub_key(&args);
				break;
			}

			default:
					printf("-> ");
					fflush(stdout);
					break;
			}
			
			free(buffer);
		}
	}
	close(sockfd);
	pthread_cancel(thread_hist);
	return 0;
}