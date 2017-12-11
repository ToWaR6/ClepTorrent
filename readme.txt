Tous les fichiers sont doté de makefile, donc un simple make dans la console devrait permettre de générer les programmes.

==============================
 Etape 1 : transfert-fichiers
==============================

1) Dans la console :
	make
1-bis) Dans la console :
	gcc server.c -o server.out
	gcc client.c -o client.out

2) Lancer le client de la sorte
	./client.out <ip> <port>
	Exemple : ./client.out localhost 1234

3) Lancer le server de la sorte
	./server.out <port>
	Exemple : ./server.out 1234

4) Le programme affichera la marche a suivre

Un programme envoie un fichier à un programme distant par un transfert TCP.

Remarque :
	- Le nom du dossier dans lequel sont stocké les fichiers reçu et envoyé sont le même 'rsc'
	- Ne pas supprimer rsc
	- Eviter de saisir un nom de fichier vide !
	- Le fichier téléchargé sera stocké dans le fichier 'rsc' avec comme nom '<nomFichier>(copie).<extension Fichier>'

========================
 Etape 2 : annuaire-p2p
========================

1) Dans la console :
	make
1-bis) Dans la console :
	gcc annuaire.c -o annuaire.out
	gcc pair.c -o pair.out -lpthread

2) Lancer l'annuaire :
	./annuaire.out <port> <nombre Pair Max>
	Exemple : ./annuaire.out 1234 3

3) Lancer un pair :
	./pair.out <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossier Source>
	Exemple : ./pair.out localhost 1234 5678 rsc
		  ./pair.out localhost 1234 8765 rsc2

4) Le programme affichera la marche a suivre

L'annuaire stocke tous les pairs connectés ainsi que leur liste de fichiers et envoie ces données à chaque nouveau pair qui se connecte.
L'utilisateur peut se synchroniser avec l'annuaire, se connecter puis se deconnecter d'un autre pair et quitter le réseaux.

Remarque :
	- Le dossier source est à passer en paramètre

========================
 Etape 4 : p2p-fichiers
========================

1) Dans la console :
	make
1-bis) Dans la console :
	gcc annuaire.c -o annuaire.out
	gcc pair.c -o pair.out -lpthread

2) Lancer l'annuaire :
	./annuaire.out <port> <nombre Pair Max>
	Exemple : ./annuaire.out 1234 3

3) Lancer un pair :
	./pair.out <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossier Source> <dossier Destination>
	Exemple : ./pair.out localhost 1234 5678 rsc1 rsc1
		  ./pair.out localhost 1234 8765 rsc2 rsc2

4) Le programme affichera la marche a suivre

En plus de l'étape précédente, les pairs peuvent échanger des fichiers lorsqu'ils sont connectés entre eux en parallele.

Remarque :
	- Les dossiers sources et destinations sont a passer en parametre
	- Fichier sources et destinations identique.

===========================================
 Etape 5 : p2p-fichiers-limiteParallelisme
===========================================

1) Dans la console :
	make
1-bis) Dans la console :
	gcc annuaire.c -o annuaire.out
	gcc pair.c -o pair.out -lpthread

2) Lancer l'annuaire :
	./annuaire.out <port> <nombre Pair Max>
	Exemple : ./annuaire.out 1234 3

3) Lancer un pair :
	./pair.out <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossier Source> <dossier Destination> <nb Max Parallele>
	Exemple : ./pair.out localhost 1234 5678 rsc1 rsc1 3
		  ./pair.out localhost 1234 8765 rsc2 rsc2 4

4) Le programme affichera la marche a suivre

Les utilisateurs peuvent maintenant envoyer plusieurs fichiers en même temps à plusieurs pairs différents.

Remarque :
	- La socket n'est pas fermer, il se peut donc que le server affiche 'bind(): adresse already use' relancer donc tout annuaire et pair
