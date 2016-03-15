all : client client2 client3 serveur

clean:
	rm -f client
	rm -f client2
	rm -f client3
	rm -f serveur

serveur : serveur.c
	gcc -o serveur serveur.c -lpthread

client : client.c
	gcc -o client client.c -lpthread

client2 : client2.c
	gcc -o client2 client2.c -lpthread

client3 : client3.c
	gcc -o client3 client3.c -lpthread