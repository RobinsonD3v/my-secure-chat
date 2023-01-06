int main()
{
    pthread_t threads[2];
    if(pthread_create(&threads [0],NULL,client_process,NULL) != 0){
	    		printf("client thread failled\n");
	    	}
	if(pthread_create(&threads [1],NULL,serveur_process,NULL) != 0){
	    		printf("serveur thread failled\n");
	    	}
	pthread_join (threads[0], NULL);
	pthread_cancel(threads[1]);

    return 0;
}
