/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include<pthread.h>
#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

int tabClient[3] = {0,0,0};
int indiceTabClient=0;

typedef struct PseudoSock PseudoSock;
struct PseudoSock{
	int socket;
	char pseudo[10];
 };
 
PseudoSock tabPseudoSock[3];// = {{0,""},{0,""}};

void *connection_handler(void *socket_desc){
	
	char buffer[256];
    int longueur;
	int sock = *(int*)socket_desc;
	char p[10];
	memset(buffer,0,sizeof(buffer));
	memset(p,0,sizeof(p));

	//Reception du pseudo client
	if(longueur = read(sock, p, sizeof(p)) > 0){
		printf("pseudo recu : %s \n",p);
		tabPseudoSock[indiceTabClient].socket=sock;
		//tabPseudoSock[indiceTabClient].pseudo="";
		strcpy(tabPseudoSock[indiceTabClient].pseudo, p);
		indiceTabClient++;
		
		//printf("premiere case pseudo = %set sock =%d\n",tabPseudoSock[0].pseudo,tabPseudoSock[0].socket);
	}

	//Reception du message envoye par le client
	while((longueur = read(sock, buffer, sizeof(buffer))) > 0 ){
		int i=0;

		printf("message recu :%s \n",buffer);
		
		//on envoie a tous les clients
		/*for(i;i<(sizeof(tabClient)/sizeof(int));i++){
			if(tabClient[i]!=sock && tabClient[i]!=0){
				write(tabClient[i], buffer, strlen(buffer));
				printf("message envoye :  %s\n", buffer);
			}
		}*/
		
		for(i;i<(sizeof(tabPseudoSock)/sizeof(PseudoSock));i++){
			if(tabPseudoSock[i].socket!=0 && tabPseudoSock[i].socket!=sock){
				write(tabPseudoSock[i].socket, buffer, strlen(buffer));
				printf("message envoye :  %s\n", buffer);
			}
		}

		memset(buffer,0,sizeof(buffer));
	}

	if(longueur == 0){

		printf("Client deconnecte\n");
		printf("sock : %d",sock);
		fflush(stdout);
	}
	else if(longueur == -1){
		
		perror("Echec de la reception du message");
	}

	//on libere la memoire
	free(socket_desc);

	return 0;
}


/*------------------------------------------------------*/

/*------------------------------------------------------*/
main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			*new_sock,
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

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
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }
    
    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);
	printf("Attente de clients...\n");

	longueur_adresse_courante = sizeof(adresse_client_courant);

    /* attente des connexions et traitement des donnees recues */
    /*for(;;) {*/
		
	/* adresse_client_courant sera renseigné par accept via les infos du connect */
	while((nouv_socket_descriptor = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) ){
		
		printf("Client connecte\n");
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = nouv_socket_descriptor;
		//tabClient[indiceTabClient]=*(int*)new_sock;
		//indiceTabClient++;

		if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) new_sock) < 0){
			
			perror("Impossible de creer le thread");
			return 1;
		}

		//pthread_join( sniffer_thread , NULL);
	}

	if (nouv_socket_descriptor < 0){
		
		perror("refus du client");
		return 1;
	}
		
	/* traitement du message */
	/*printf("reception d'un message.\n");
	close(nouv_socket_descriptor);*/
		
    return 0;
    
}

