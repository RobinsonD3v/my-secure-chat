#include "chat.h"
//c'est ici que tout vas se lancer

int main(){
	printf("bienvenue sur mon chat !!!\n\n");
	//serveur();
	user user1;
    user user2;
	
	
	

	user1.share = 'y';
	user1.online = 'n';
	user1.del = 'n';

	strcpy(user1.id,"ed850d77-1ae1-4d3a-93c3-6e2663ff7782");
	strcpy(user1.ip,"192.168.1.80");
	strcpy(user1.pseudo,"bob");
	
	//add_user(user1);

	display_userlist();
	
	client();
	return 0;
}