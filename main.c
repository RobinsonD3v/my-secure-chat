#include "chat.h"
#include <pthread.h>

int main()
{
	printf("\n");
	printf("                    /\\_/\\           ___\n");
	printf(" My Secure Chat    = o o =_______    \\ \\\n");
	printf("  by Robinson       \\_-     __(  \\.__) )\n");
	printf("                   <_____>__(_____)____/\n");

    pthread_t threads[2];
    if(pthread_create(&threads [0],NULL,client_process,NULL) != 0){
	    		print_log("client : thread failled\n");
	    		printf("une erreur est survenue\n");
	    	}
	if(pthread_create(&threads [1],NULL,serveur_process,NULL) != 0){
	    		print_log("serveur : thread failled\n");
	    		printf("une erreur est survenue\n");
	    	}

	pthread_join (threads[0], NULL);
	pthread_cancel(threads[1]);
	print_log("serveur: stop\n");
	
    return 0;
}

//le makefile et threadmax
