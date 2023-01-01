#include "chat.h"

/*
liste des commandes classique

new -> nvelle conv
msg -> nvelle conv
add -> ajoute
clear [user] -> enlève le user
clear_db -> nettoie la db
me -> mes infos
info [user] -> infos de l'utilisateur
clear_history -> efface l'historique
display_db -> affiche les contacts
display_online -> affiche les gens en ligne
modify_user -> modifier les info d'un user
modify_me -> modifiers mes info
send_presence -> envoie que je suis en ligne
exit -> quite l'app

help -> affiche ça

todo :

	toujours un pb avec le exit / arg dans client2
	le pb avec msg quand il n'y as pas d'arg

//opérations sur les séries divergentes
*/
#define NBR_CMD 20
struct commande_st{
	char *name;
	char *help;
	char *syntaxe;
	char type;
};

struct commande_st commandes[] = {
	{"new","nouvelle conversation alias : msg,join","new [pseudo]",'m'},//0                 
	{"msg","nouvelle conversation alias : new,join","msg [pseudo]",'m'},
	{"join","nouvelle conversation alias : new,msg","join [pseudo]",'m'},
	{"file", "envoie un fichier (nécésite un conversation","file",'m'},

	{"add","ajoute un nouvelle utilisateur", "add",'u'},//4
	{"remove","efface un utilisateur","remove <user id>",'u'},
	{"info","affiche les infos d'un utilisateur","info <user id>",'u'},
	{"me","affiche les infos sur toi","me",'u'},
	{"modify","modifie les informations d'un utilisateur","modify <user id>",'u'},
	{"modify_me","modifie tes informations","modify_me",'u'},

	{"clear_db","nettoie définitivement la base de donnée utilisateur","clear_db",'b'},//10
	{"clear_history","efface l'historique de la conversation","clear_history [id]",'b'},
	{"contactes","affiche la base de donnée utilisateur","contactes [nbr à afficher]",'b'},
	{"online","affiche les utilisateur en ligne","display_online",'b'},
	
	{"set_presence","permet de faire savoir que tu es en ligne","set_presence <y/n>",'p'},//14
	{"get_presence","permet de savoir qui est en ligne","get_presence"},
	{"send_contact","envoie aléatoirement X de tes contactes","send_contact [nbr de contacte (défaut 10)]",'p'},

	{"exit","quite l'application","exit",'s'},//16
	{"clear","efface l'écran","clear",'s'},
	{"help","affiche l'aide, pour obtenir la syntaxe fait help [commande]","help [commandes]",'s'},
	{"key", "génère une nouvelle clé","key",'s'}//20
};
/*ctx : 
   0 -> accueil
   1 -> message
   2 -> les deux
*/

typedef struct Parsed Parsed;

struct Parsed{
	char *cmd;
	Parsed *next;
};
typedef struct Parsed_list Parsed_list;

struct Parsed_list{
	Parsed *first;
};

void insertion(Parsed_list *parsed_list, char *new){
	Parsed *new_elem = malloc(sizeof(*new_elem));

	if (parsed_list == NULL || new == NULL)
	{
		exit(1);
	}

	new_elem->cmd = malloc(strlen(new)+1);
	strcpy(new_elem->cmd,new);

	new_elem->next = parsed_list->first;
	parsed_list->first = new_elem;
}

void clean_parsed(Parsed_list *list){
	while(list->first != NULL){
		Parsed *current = malloc(sizeof(*current));
		current = list->first;
		list->first = current->next;
		free(current);
	}
	free(list);
}
int parse_command(char *cmd, int context, user *recipient){

	if (!cmd[0])
		return -1;

	if (cmd[0] == '/' || context == 0)
	{

		Parsed_list *parsed_list = malloc(sizeof(*parsed_list));
		Parsed *parsed = malloc(sizeof(*parsed));

		if (parsed_list == NULL || parsed == NULL)
		{
			printf("\nerreur allocation\n\n");
			return -1;
		}
		parsed->next = NULL;
		parsed_list->first = parsed;

		long int len = strlen(cmd);

		int init_size = len;

		char delim[] = " /";

		char *ptr = strtok(cmd, delim);

		int index = 0;
		while(ptr != NULL)
		{	
			insertion(parsed_list,ptr);
			
			ptr = strtok(NULL, delim);
			index++;
		}

		Parsed *current = parsed_list->first;
		
		for (int i = 0; i < index-1; ++i)
		{
			current = current->next;
		}
			
		if (current->cmd)
		{
			int result = -1;	
			for (int j = 0; j <= NBR_CMD; ++j)
			{
				
				if (strcmp(current->cmd,commandes[j].name)  == 0)
				{
					result = j;
					break;
				}
			}

			switch(result){

				case 0://new
				case 1://msg
				case 2://join
				{
					if (context == 1)
					{
						printf("\ntu dois quitter la conversation pour en démarer une nouvelle\n\n");
						break;
					}
					user recipient;
					if (index > 1)
					{
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;

						recipient = get_user(arg->cmd);
					}
					else{
						char *buffer = malloc(34);
						printf("pseudo de la personne à contacter : ");
						fgets(buffer,34,stdin);
						buffer[strlen(buffer)-1] = '\0';
						recipient = get_user(buffer);

						free(buffer);
					}
					if (recipient.pseudo[0] == 0)
					{
						printf("utilisateur introuvable\n\n");
						break;
					}

					if(client2(recipient,&send_msg,&afficher_historique,&parse_command) != 0){
						printf("une erreur c'est produite\n\n");
						return 1;
					}
					break;
				}
				case 3:{
					return 3;
				}
				case 4:{//add    faire une verif que les infos sont bonnes  creer une cmd check
					user new;
					char *buffer = malloc(39);
					printf("ajout d'un nouvelle utilisateur :\n\n");
					printf("pseudo : ");
					fgets(buffer,34,stdin);
					buffer[strlen(buffer)-1] = '\0';
					strcpy(new.pseudo,buffer);
					memset(buffer,0,39);

					printf("ip : ");
					fgets(buffer,17,stdin);
					buffer[strlen(buffer)-1] = '\0';
					strcpy(new.ip,buffer);
					memset(buffer,0,39);

					printf("id : ");
					fgets(buffer,38,stdin);
					buffer[strlen(buffer)-1] = '\0';
					strcpy(new.id,buffer);
					memset(buffer,0,39);
					
				    new.share = 'y';
				    new.del = 'n';

				    if(add_user(new) != 0){
				    	printf("une erreur lors de la création de l'utilisateur a eu lieu\n");
				    }
				    free(buffer);
					break;
				}
				case 5://remove
					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;
						switch(del_user(arg->cmd)){
							case 0:
								printf("utilisateur suprimmer, pense à nettoyer la base de donnée.\n");
								break;
							case 1:
								printf("cette utilisateur n'existe pas\n");
								break;
							default:
								printf("une erreur a eu lieu\n");
								break; 
						}			
					}
					else{
						printf("il faut spécifier l'id de l'utilisateur à suprimmer\n");
					}
					break;

				case 6:{//info
					user utilisateur;

					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;
						printf("\n");
						utilisateur = get_user(arg->cmd);
						
					}
					else if(context == 1){
						
						if (!recipient)
							break;

						utilisateur = get_user(recipient->pseudo);	
					}
					else{
						printf("\nspécifier un utilisateur\n\n");
						break;
					}

					if (!utilisateur.pseudo[0])
					{
						printf("utilisateur inexisant\n\n");
						break;
					}

					printf("pseudo : %s\nid : %s\nip : %s\n",utilisateur.pseudo,utilisateur.id,utilisateur.ip);
					if (utilisateur.online == 'y')
					{
						printf("en ligne\n\n");
					}
					else{
						printf("hors-ligne\n");
					}

					printf("share : %c\n\n",utilisateur.share);
					break;
				}
				case 7:{//me
					extend_user me = get_you();

					if (me.user.pseudo[0])
					{
						printf("\npseudo : %s\nid : %s\nip : %s\n",me.user.pseudo,me.user.id,me.user.ip);
						if (me.user.online == 'y')
						{
							printf("en ligne\n\n");
						}
						else if(me.user.online == 'n'){
							printf("hors-ligne\n\n");
						}
					}
					else{
						printf("\nune erreur est survenue %s\n\n",me.user.id);
					}
					break;
				}

				case 8://modify
					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;

						user new;
						user current;

						current = get_user_by_id(arg->cmd);

						if (current.pseudo[0] == 0)
						{
							printf("l'utilisateur n'éxiste pas\n\n");
							break;
						}

						new.share = 'y';
						new.del = current.del;

						char *buffer = malloc(34);
						memset(buffer,0,34);
						printf("nouveau pseudo : ");
						fgets(buffer,33,stdin);
						buffer[strlen(buffer)-1] = '\0';
						strcpy(new.pseudo,buffer);
						memset(buffer,0,34);

						printf("nouvel ip : " );
						fgets(buffer,17,stdin);
						buffer[strlen(buffer)-1] = '\0';
						strcpy(new.ip,buffer);
						free(buffer);

						strcpy(new.id,arg->cmd);
						
						switch(modify_user(arg->cmd,new)){
							case 0 : 
								printf("utilisateur modifié avec succès\n\n");
								break;
							case 1:
								printf("\nl'utilisateur n'éxiste pas\n\n");
								break;
							default:
								printf("une erreur est survenue");
								break;
						}
					}
					else{;
						printf("\nspécifie un identifiant\n\n");
					}
					break;

				case 9:{//modify_me

					extend_user old = get_you();
					char *buffer = malloc(34);
					printf("\nmot de passe : ");
					fgets(buffer,31,stdin);
					buffer[strlen(buffer)-1] = '\0';

					//pensé à utiliser des hash

					if (strcmp(buffer,old.pass) == 0)
					{
						extend_user new;
						memset(buffer,0,34);
						printf("nouveau pseudo : ");
						fgets(buffer,33,stdin);
						buffer[strlen(buffer)-1] = '\0';
						strcpy(new.user.pseudo,buffer);
						memset(buffer,0,34);

						printf("nouvel ip : " );
						fgets(buffer,16,stdin);
						buffer[strlen(buffer)-1] = '\0';
						strcpy(new.user.ip,buffer);

						strcpy(new.user.id,old.user.id);

						printf("partager automatiquement ton profile? [Y/n] ");
					    fgets(buffer,3,stdin);
					    if (buffer[0] != 'n')
					    {
					    	buffer[0] = 'y';
					    }
					    new.user.share = buffer[0];
					    memset(buffer,0,37);

					    printf("nouveau mot de passe (max 30 caractères) : ");
	    				fgets(buffer,31,stdin);
	    				buffer[strlen(buffer)-1] = '\0';
	    				strcpy(new.pass,buffer);

	    				switch(modify_you(new)){
		    				case 0:
		    					printf("\nmodification réussie\n\n");
		    					break;
		    				case 1:
		    					printf("\nles id ne correspondent pas\n\n");
		    					break;
		    				default:
		    					printf("\nune erreur est survenue\n\n");
		    					break; 
	    				}
					}
					else{
	    					printf("\nmot de passe incorrecte\n\n");
	    			}

	    			free(buffer);
					break;
				}

				case 10://clear_db
					if(clean_userlist() != 0){
						printf("\nune erreur est survenue\n\n");
					}
					break;

				case 11://clear_history

					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;

						char *path = "conv/";
						char *filePath = malloc(strlen(path)+strlen(arg->cmd)+1);
						strcpy(filePath,path);
						strcat(filePath,arg->cmd);

						FILE *fichier = fopen(filePath,"w");
				    	if (!fichier)
				    	{
				    		printf("l'historique de conversation non trouvé\n");
				    		break;
				    	}
				    	printf("\nhistorique effacé\n\n");
				    	fclose(fichier);
					}
				else if(context == 1){
					//récup l'utilisateur à qui je parle
					if (recipient)
					{
						char *path = "conv/";
						char *filePath = malloc(strlen(path)+strlen(recipient->id)+1);
						strcpy(filePath,path);
						strcat(filePath,recipient->id);

						FILE *fichier = fopen(filePath,"w");
				    	if (!fichier)
				    	{
				    		printf("l'historique de conversation non trouvé\n");
				    		break;
				    	}
				    	printf("\nhistorique effacé\n\n");
				    	fclose(fichier);
					}
				}
				else{
					printf("\nindique l'id de l'utilisateur duquel tu veux supprimer l'historique\n\n");
				}
				break;
				case 12:{//contactes
					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;

						if(strcmp(arg->cmd,"all") == 0 || arg->cmd[0] == 'a'){
							if(display_userlist('y') != 0){
								printf("\nune erreur a eu lieu lors de l'affichage des contactes\n\n");
							}
						}
						else{
							if(display_userlist('n') != 0){
								printf("\nune erreur a eu lieu lors de l'affichage des contactes\n\n");
							}
						}
						
					}
					else{
						if(display_userlist('n') != 0){
							printf("\nune erreur a eu lieu lors de l'affichage des contactes\n\n");
						}
					}
					break;
				}
				case 13://display_online
					if(display_online() != 0){
						printf("\nune erreur est survenue\n\n");
					}
					break;

				case 14:{//set prescence
					char *buffer = malloc(3);
					printf("tu veux apparraître en ligne ? [Y/n] : ");
					fgets(buffer,3,stdin);

					if (buffer[0] != 'n')
					{	
						buffer[0] = 'y';
					}

					extend_user me = get_you();
					me.user.online = buffer[0];
					if (modify_you(me) != 0)
					{
						printf("une erreur est survenue\n\n");
						break;
					}
					printf("status mis à jour avec succès\n\n");
					free(buffer);
					break;
				}
				case 15://get_presence
					{
						if(get_presence() != 0){
							printf("une erreur c'est produite\n\n");
						}
						break;
					}
				case 16://send_contact
					return 4;
					break;

				case 17: //exit
					return 1;
					break;
				case 18: //clear
					clear();
					break;

				case 19: //help
					if(index > 1){
						Parsed *arg = malloc(sizeof(*arg));
						arg = parsed_list->first;

						char find = 'n';
						for (int j = 0; j <= NBR_CMD; ++j)
							{
								if (strcmp(arg->cmd,commandes[j].name) == 0)
								{
									printf("\n%s : %s\nsyntaxe : %s\n\n",commandes[j].name,commandes[j].help,commandes[j].syntaxe);
									find = 'y';
									break;
								}
							}
						if (find == 'n')
						{
							printf("\n%s : cette commande n'éxiste pas.\n\n",arg->cmd);
						}
					}
					else{
						printf("\npour faire une commande :\n");
						printf("\t <commande> [args] -> accueil\n");
						printf("\t/<commande> [args] -> message\n\n");
						printf("liste des commmandes :\n\n");
						char previous = commandes[0].type;

						for (int j = 0; j <= NBR_CMD; ++j)
						{
							if (commandes[j].type != previous)	
							{
								printf("\n");
								previous = commandes[j].type;
							}
							printf("\t%s : %s\n",commandes[j].name,commandes[j].help);
						}
						printf("\nnote : [args] -> facultatif, <args> -> obligatoire\n\n");
					}
					break;
				case 20:
					if (context == 1)
					{
						printf("génère une nouvelle clée\n");
						return 5;
					}
					else{
						printf("lance une nouvelle conversation pour changer de clée\n");
					}			
					break;
				default:
					printf("%s: commande introuvable\n",current->cmd);
					break;
			}
		}
		else{
			printf("une erreur a eu lieu\n");
		}

		clean_parsed(parsed_list);
	
	}
	else{
		return 2;
	}

	return 0;
   
}

int main()
{
	while(1){
		struct args_parse{
			char cmd[1001];
			int context;
		}args_parse;

		printf(">>> ");
		char *cmd = malloc(1001);
		fgets(cmd,1001,stdin);
		cmd[strlen(cmd)-1] = '\0';
		strcpy(args_parse.cmd,cmd);
		args_parse.context = 0;
		
		
		switch(parse_command(cmd,0,NULL)){
			case 1:
				printf("\nÀ bientôt !\n\n");
				return 0;
			case 4:{
				;
			}
		}

		memset(cmd,0,1001);
		free(cmd);
	}
	return 0;
}