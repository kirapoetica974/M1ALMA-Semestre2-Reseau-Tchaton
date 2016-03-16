/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include<pthread.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

void viderBuffer()

{

    int c = 0;

    while (c != '\n' && c != EOF)

    {

        c = getchar();

    }

}

void *connection_handler(void *socket_desc){
	
	char mesg[256];
	int sock = *(int*)socket_desc;
	printf("Ecrivez votre message...\n");

	while(1)
	{
		memset(mesg,0,sizeof(mesg));
		fgets(mesg, sizeof mesg, stdin);
		
		printf("Envoi d'un message au serveur... \n");

		//envoi des données au serveur
		if ((write(sock, mesg, strlen(mesg))) < 0) {
			perror("erreur : impossible d'ecrire le message destine au serveur.");
			exit(1);
    	}
		
		//sleep(3);
    	printf("Message envoye au serveur.\n");
		//memset(mesg,0,sizeof(mesg));
		printf("Ecrivez votre message...\n");
	}
	
	//on libere la memoire
	free(socket_desc);
	return 0;
}



int main(int argc, char **argv) {
  
    int 	socket_descriptor, 	/* descripteur de socket */
			*new_sock,
		longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	buffer[256];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
    //char mesg[255]; 			/* message envoyé */
     
    if (argc != 2) {
	perror("Usage : client <adresse-serveur>");
	exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    
    printf("- Nom de l'executable : %s \n", prog);
    printf("- Adresse du serveur  : %s \n", host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
	perror("erreur : impossible de recuperer le numero de port du service desire.");
	exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("- Numero de port pour la connexion au serveur : %d \n\n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("erreur : impossible de creer la socket de connexion avec le serveur.");
	exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
	perror("erreur : impossible de se connecter au serveur.");
	exit(1);
    }
    
    printf("Connexion etablie avec le serveur :) \n\n");

	char pseudo[10];
	int pseudoCorrect=0;
	int i;
	int pseudoLibre=0;

	//Pseudo libre ?
	while(pseudoLibre==0){
		//Controle du pseudo (pas de caracteres spéciaux et pseudo différent de 'q'
		while(pseudoCorrect==0){
			memset(pseudo,0,sizeof(pseudo));
			printf("Quel est votre pseudo ? (< 10 et sans espace) : ");
			fgets(pseudo, 10, stdin);
		
			if(strchr(pseudo,'\n')==NULL){
				viderBuffer();
				printf("Pseudo trop long !\n");
			}
			else{
				if(strcmp(pseudo,"q\n")==0 || strchr(pseudo,'/')!=NULL || strchr(pseudo,' ')!=NULL){
					printf("Pseudo incorrect !\n");
				}
				else pseudoCorrect=1;
			}
		}
		
		//Envoi du pseudo
		if ((write(socket_descriptor, pseudo, strlen(pseudo))) < 0) {
			perror("erreur : impossible d'envoyer le pseudo au serveur.");
			exit(1);
		}
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
			if(strcmp(buffer,"Pseudo Ok.\n")){
				write(1,buffer,longueur);
				pseudoLibre=1;
			}	
		}
	}

	pthread_t sniffer_thread;
	new_sock = malloc(1);
	*new_sock = socket_descriptor;

	if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) new_sock) < 0){		
		perror("Impossible de creer le thread");
		return 1;
	}

	while(1){
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
			write(1,buffer,longueur);
			memset(buffer,0,sizeof(buffer));
			printf("Ecrivez votre message...\n");
		}
    }

	printf("\nfin de la reception.\n");
	close(socket_descriptor);
	return 0;
                
    
    //close(socket_descriptor);
    
    //printf("connexion avec le serveur fermee, fin du programme.\n");
    
    //exit(0);
    
}
