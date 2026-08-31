# Fonctionnement du programme `ft_irc`

Ce document explique le serveur avec des mots simples. Le but est de pouvoir
suivre le chemin d'une connexion, depuis le lancement du programme jusqu'au
traitement des commandes IRC.

## 1. Idée générale

Le programme est un serveur IRC écrit en C++98.

Il utilise :

- un socket TCP pour recevoir les connexions ;
- des sockets non bloquants pour les clients ;
- un seul appel à `poll()` dans la boucle principale ;
- un buffer d'entrée pour chaque client ;
- une file de sortie pour chaque client.

Le serveur ne bloque pas sur un client. Si un client ne parle pas, ou s'il ne
lit pas ses réponses, les autres clients continuent à fonctionner.

## 2. Lancement

Le serveur se lance avec deux arguments :

```bash
./ircserv <port> <mot_de_passe>
```

Exemple :

```bash
./ircserv 6667 secret
```

Dans `main.cpp`, le programme :

1. vérifie le nombre d'arguments ;
2. refuse un mot de passe vide ;
3. ignore `SIGPIPE` pour éviter un arrêt brutal pendant un `send()` ;
4. prépare `SIGINT` et `SIGTERM` pour arrêter le serveur proprement ;
5. crée le socket d'écoute ;
6. lance la boucle principale ;
7. ferme le socket d'écoute à la fin.

## 3. Création du socket serveur

La fonction `create_listening_socket()` fait le travail réseau initial.

Elle :

1. vérifie que le port est un nombre entre `1` et `65535` ;
2. crée un socket TCP IPv4 avec `socket(AF_INET, SOCK_STREAM, 0)` ;
3. active `SO_REUSEADDR` ;
4. écoute sur toutes les interfaces avec `INADDR_ANY` ;
5. fait le `bind()` ;
6. met le socket en non bloquant avec :

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

7. lance `listen()`.

Si une étape échoue, le socket est fermé et le programme retourne une erreur.

## 4. Boucle principale

La boucle principale est dans `boucle_principale.cpp`.

Elle contient un vecteur de `pollfd` :

```text
fds[0] = socket serveur
fds[1] = client 1
fds[2] = client 2
...
```

Le socket serveur attend `POLLIN`, car il sert à accepter de nouveaux clients.

Chaque client attend :

- `POLLIN` quand on peut lire des données ;
- `POLLOUT` seulement quand il y a une réponse à envoyer.

Le serveur utilise un seul appel à `poll()` :

```cpp
poll(&fds[0], fds.size(), -1)
```

Ensuite, il regarde les événements reçus.

## 5. Événements gérés

| Événement | Action |
|---|---|
| `POLLIN` sur le socket serveur | accepter un nouveau client avec `accept()` |
| `POLLIN` sur un client | lire avec `recv()` |
| `POLLOUT` sur un client | envoyer une partie de la file avec `send()` |
| `POLLERR` | déconnecter le client |
| `POLLNVAL` | retirer le descripteur invalide |
| `POLLHUP` | gérer une fermeture de connexion |

`accept()`, `recv()` et `send()` sont donc appelés seulement après `poll()`.

## 6. Arrivée d'un client

Quand un nouveau client arrive :

1. `poll()` signale `POLLIN` sur le socket serveur ;
2. le serveur appelle `accept()` ;
3. le nouveau socket est mis en non bloquant ;
4. un objet `Client` est créé dans la base de données ;
5. le client est ajouté dans le vecteur de `pollfd`.

Le serveur n'envoie pas de message spécial au moment de la connexion. Il attend
les commandes IRC du client.

## 7. Données d'un client

Chaque client garde :

- son nickname ;
- son username ;
- son realname ;
- l'état de `PASS` ;
- l'état de `NICK` ;
- l'état de `USER` ;
- son état d'enregistrement ;
- son buffer d'entrée ;
- sa file de sortie ;
- la liste des canaux rejoints.

Un client devient enregistré seulement quand il a envoyé :

```text
PASS <mot_de_passe_correct>
NICK <nickname>
USER <username> 0 * :<realname>
```

L'ordre peut changer. La fonction `try_registration()` vérifie si les trois
conditions sont remplies.

## 8. Lecture des commandes

Quand un client a `POLLIN`, le serveur appelle `recv()` une seule fois.

Les données reçues sont ajoutées au buffer d'entrée du client.

Ensuite, le serveur cherche des lignes complètes terminées par `\n`.

Exemple avec une commande coupée en deux paquets :

```text
Paquet 1: PRIV
Paquet 2: MSG #general :salut\r\n
```

Le buffer reconstruit :

```text
PRIVMSG #general :salut\r\n
```

La ligne complète est envoyée au parseur IRC. Le morceau incomplet reste dans le
buffer jusqu'au prochain `recv()`.

## 9. Parseur IRC

Le parseur lit une ligne IRC et sépare :

- le préfixe optionnel ;
- la commande ;
- les paramètres ;
- le dernier paramètre après `:`.

Exemple :

```text
PRIVMSG #general :bonjour tout le monde
```

Résultat :

```text
commande = PRIVMSG
paramètre 1 = #general
paramètre 2 = bonjour tout le monde
```

Les commandes sont converties en majuscules. Donc `nick`, `Nick` et `NICK` sont
traités comme la même commande.

## 10. Écriture des réponses

Les fonctions de commandes n'appellent pas `send()` directement.

Elles ajoutent les réponses dans la file de sortie du client.

Quand cette file n'est pas vide, le client reçoit l'événement `POLLOUT`.

Quand `poll()` signale `POLLOUT`, le serveur appelle `send()`.

Si seulement une partie du message est envoyée, le reste reste dans la file pour
plus tard.

La file est limitée. Si un client ne lit jamais ses messages et que la file
devient trop grande, le client est déconnecté.

## 11. Commandes de base

Le serveur gère les commandes principales :

- `PASS` : vérifie le mot de passe ;
- `NICK` : définit ou change le nickname ;
- `USER` : définit les informations utilisateur ;
- `PING` : répond avec `PONG` ;
- `QUIT` : ferme la connexion proprement ;
- `JOIN` : rejoint un canal ;
- `PART` : quitte un canal ;
- `PRIVMSG` : envoie un message à un utilisateur ou à un canal.

Il gère aussi des commandes utiles pour les clients IRC :

- `CAP` ;
- `NAMES` ;
- `WHO` ;
- `NOTICE`.

## 12. Canaux

Un canal est créé automatiquement au premier `JOIN`.

Le premier client du canal devient opérateur.

Un canal garde :

- son nom ;
- ses membres ;
- ses opérateurs ;
- les utilisateurs invités ;
- son topic ;
- ses modes.

Quand un canal n'a plus de membres, il est supprimé.

## 13. Messages dans un canal

Quand un client envoie :

```text
PRIVMSG #general :salut
```

Le serveur :

1. vérifie que le canal existe ;
2. vérifie que l'expéditeur est membre du canal ;
3. envoie le message aux autres membres du canal ;
4. n'envoie pas le message à l'expéditeur.

## 14. Opérateurs et modes obligatoires

Les commandes obligatoires sont présentes :

- `KICK` : retirer un utilisateur du canal ;
- `INVITE` : inviter un utilisateur ;
- `TOPIC` : lire ou modifier le sujet du canal ;
- `MODE` : modifier les modes du canal.

Modes gérés :

| Mode | Effet |
|---|---|
| `+i` / `-i` | canal sur invitation seulement |
| `+t` / `-t` | seul un opérateur peut changer le topic |
| `+k` / `-k` | ajouter ou retirer une clé de canal |
| `+o` / `-o` | donner ou retirer le statut opérateur |
| `+l` / `-l` | ajouter ou retirer une limite d'utilisateurs |

Un utilisateur normal ne peut pas exécuter les actions réservées aux
opérateurs.

## 15. Déconnexion

Un client peut partir avec `QUIT`, ou disparaître brutalement.

Dans les deux cas, le serveur :

1. retire le client de la base de données ;
2. le retire de tous les canaux ;
3. prévient les membres concernés si le client était enregistré ;
4. supprime les canaux devenus vides ;
5. ferme le descripteur du socket.

## 16. Résumé pour la défense

Phrase simple à retenir :

```text
Le serveur utilise un seul poll pour surveiller le socket serveur et tous les
clients. accept, recv et send sont appelés seulement après les événements de
poll. Chaque client a un buffer d'entrée pour les commandes partielles et une
file de sortie pour éviter de bloquer sur l'écriture.
```
