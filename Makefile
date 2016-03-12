all : client client2 serveur

clean:
	rm -f client
	rm -f client2
	rm -f serveur

serveur : serveur.c
	gcc -o serveur serveur.c -lpthread

client : client.c
	gcc -o client client.c

client2 : client2.c
	gcc -o client2 client2.c
