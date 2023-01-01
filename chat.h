#ifndef _CHAT_
#define _CHAT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include <uuid/uuid.h>
/*
todo : 
	   l'overflow avec le nbr de thread
       
       reorganiser 
       
       le parsing
	   
	   chiffrer

	   faire une lib

court terme :
	   chiffrement
	   		-> ya un pb sans doute avec la taille (aléatoire != 1015);
	   		    overflow ???

	   parse_serv est vraiment deg
	   faire le client général

	   puis passer au todo !!!
*/

//4e807496-065a-469d-96f2-9722d5513f35

/*
0 -> msg normal
1 -> contactes
2 -> prescence
3 -> fichier

client dédié:
4 -> demande clée
5 -> clée asymétrique

*/
//constante

#define LEN_ID 37
#define clear() printf("\x1B[2J\x1B[H")
#define SA struct sockaddr
#define PORT 8080
#define ADDITIONAL_DATA (const unsigned char *) "robinsond3v"
#define ADDITIONAL_DATA_LEN 11
#define CRYPT_MESSAGE_LEN 999

//variable

typedef struct{
	char id[LEN_ID];
	char ip[17];
	char pseudo[32];
	char share;
	char online;
	char del;
}user;

typedef struct{ // rajouter heure/minute
	int jour;
	int mois;
	int annee;
}temps;

typedef struct{
	int type;            
	char dest[LEN_ID];   
	user author;    //changer par author       
	temps date;          
	char content[1016];
	unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
	int securplus;
	
}msg;

typedef struct
	{
		user user;
		char pass[257];
}extend_user;

//fonction

int serveur(void);
int client(void);
void *server_thread(void *arg);
temps get_date(void);
void *afficher_historique(void *args);
user get_user(char *pseudo);
int add_user(user utilisateur);
int del_user(char *id);
int clean_userlist(void);
int display_userlist(char all);
int modify_user(char *id,user new);
int get_user_index(char *id);
user get_user_by_index(int index);
user get_user_by_id(char *id);
int total_user(void);
int make_you(user you,char pass[257]);
extend_user get_you(void);
int display_online(void);
int modify_you(extend_user you);
int client2(user recipient,int (*send_function)(void *),void *(*history_function)(void *args),int (*parse_function)(char *,int,user *));
int send_msg(void *args);
int send_file(void *argp);
int get_presence(void);
int parse_command(char *cmd, int context,user *recipient);
int parse_serv(msg message,int *sock);

#endif
