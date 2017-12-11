Tous les fichiers sont doté de makefile, donc un simple make dans la console devrait permettre de générer les programmes

Etape 1 : transfert-fichiers
==================================

1) Dans la console : 
	make 
ou 
	gcc server.c -o server.out
	gcc client.c -o client.out
2) Lancer le client de la sorte
	./client.out <ip> <port>
	Exemple : ./client.out localhost 1234
3) Lancer le server de la sorte 
	./server.out <port>
	Exemple : ./server.out 1234

4) Le programme affichera la marche a suivre

Remarque : 
	- Le nom du dossier dans lequel sont stocké les fichiers reçu et envoyé sont le même 'rsc'
	- Ne pas supprimer rsc 
	- Eviter de saisir un nom de fichier vide !
	- Le fichier téléchargé sera stocké dans le fichier 'rsc' avec comme nom '<nomFichier>(copie).<extension Fichier>'

Etape 2 : annuaire-p2p
==================================
1) Dans la console : 
	make
ou
	gcc annuaire.c -o annuaire.out
	gcc pair.c -o pair.out -lpthread

2) Lancer l'annuaire :
	./annuaire.out <port> <nombre Pair Max>
3) Lancer un pair :
	./pair.out <IP_SERV> <PORT_SERV> <PORT_CLIENT> <dossier Source> 
4) Le programme affichera la marche a suivre

Remarque : 
	- Le dossier source est à passer en paramètre 
	- 


