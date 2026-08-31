# Audit du projet `ft_irc`

Ce document sert de checklist avant une évaluation. Il ne remplace pas les tests
pendant la soutenance, mais il aide à voir rapidement si la partie obligatoire
est prête.

## 1. Résultat général

La partie obligatoire semble cohérente avec le sujet `ft_irc`.

Le projet :

- compile en C++98 ;
- produit l'exécutable `ircserv` ;
- utilise un seul `poll()` ;
- utilise des sockets non bloquants ;
- gère plusieurs clients ;
- gère les commandes IRC obligatoires ;
- ne montre pas de fuite mémoire dans le test Valgrind exécuté.

Les bonus ne sont pas pris en compte ici.

## 2. Compilation

| Point vérifié | État |
|---|---|
| `Makefile` présent | OK |
| règle `all` | OK |
| règle `clean` | OK |
| règle `fclean` | OK |
| règle `re` | OK |
| exécutable nommé `ircserv` | OK |
| compilation avec `-Wall -Wextra -Werror` | OK |
| compilation en C++98 | OK |

Commande testée :

```bash
make -B
```

Résultat : compilation réussie.

## 3. Points éliminatoires du sujet

### `poll()`

Il y a un seul appel réel à `poll()` dans le code :

```cpp
poll(&fds[0], fds.size(), -1)
```

Il se trouve dans la boucle principale.

### `accept()`, `recv()` et `send()`

Le flux est correct :

- `accept()` est appelé après `POLLIN` sur le socket serveur ;
- `recv()` est appelé après `POLLIN` sur un client ;
- `send()` est appelé après `POLLOUT` sur un client.

Le code ne relance pas une opération selon `errno == EAGAIN`.

### `fcntl()`

Les appels à `fcntl()` utilisent la forme autorisée :

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

Cela est fait pour :

- le socket d'écoute ;
- chaque nouveau socket client.

## 4. Démarrage du serveur

Le serveur attend :

```bash
./ircserv <port> <password>
```

Il vérifie :

- le nombre d'arguments ;
- la validité du port ;
- le mot de passe non vide.

Le serveur écoute avec `INADDR_ANY`, donc sur toutes les interfaces réseau de la
machine.

## 5. Connexion avec `nc`

Test de base :

```bash
nc 127.0.0.1 6667
```

Commandes envoyées :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
```

Résultat attendu :

```text
:ircserv 001 alice :Welcome ...
```

Ce test a fonctionné avec des clients TCP automatisés.

## 6. Plusieurs clients

Le serveur accepte plusieurs clients en même temps.

Tests effectués :

- deux clients enregistrés en même temps ;
- un client envoie un message pendant qu'un autre reste connecté ;
- un client rejoint un canal ;
- un autre client rejoint le même canal ;
- les messages envoyés au canal sont reçus par les autres membres.

Résultat : OK.

## 7. Commandes partielles

Le serveur garde les morceaux incomplets dans le buffer du client.

Exemple testé :

```text
PASS pa
ss
```

Le serveur reconstruit la commande complète avant de la traiter.

Pendant qu'un client garde une commande incomplète, les autres clients
continuent à fonctionner.

Résultat : OK.

## 8. Déconnexions inattendues

Tests effectués :

- fermeture d'un client enregistré ;
- fermeture d'un client avec une commande incomplète ;
- connexion d'un nouveau client après cette fermeture ;
- `PING` envoyé par un client restant.

Résultat : le serveur reste opérationnel.

## 9. Mémoire

Test Valgrind exécuté sur un scénario avec deux clients :

- enregistrement ;
- `JOIN` ;
- `PRIVMSG` ;
- `QUIT` ;
- arrêt du serveur avec `SIGINT`.

Résultat Valgrind :

```text
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors
```

## 10. Commandes IRC testées

| Commande | État |
|---|---|
| `PASS` | OK |
| `NICK` | OK |
| `USER` | OK |
| `PING` / `PONG` | OK |
| `QUIT` | OK |
| `JOIN` | OK |
| `PART` | présent dans le code |
| `PRIVMSG` utilisateur | OK |
| `PRIVMSG` canal | OK |
| `CAP` | présent pour compatibilité client |
| `NAMES` | présent |
| `WHO` | présent |
| `NOTICE` | présent |

## 11. Opérateurs

Commandes obligatoires :

| Fonction | État |
|---|---|
| `KICK` | OK |
| `INVITE` | OK |
| `TOPIC` | OK |
| `MODE +i` / `-i` | OK |
| `MODE +t` / `-t` | OK |
| `MODE +k` / `-k` | OK |
| `MODE +o` / `-o` | OK |
| `MODE +l` / `-l` | OK |

Un utilisateur normal reçoit une erreur quand il essaie une action réservée aux
opérateurs.

Attention : un utilisateur normal peut changer le topic si le mode `+t` n'est
pas actif. Quand `+t` est actif, seul un opérateur peut le faire.

## 12. Points à refaire pendant la défense

Avant de valider le projet, refaire manuellement :

1. cloner le dépôt dans un dossier vide ;
2. vérifier que le dépôt est le bon ;
3. lancer `make`;
4. lancer `./ircserv 6667 secret`;
5. connecter deux clients avec `nc`;
6. tester `PASS`, `NICK`, `USER`;
7. tester `JOIN #test`;
8. tester `PRIVMSG #test :hello`;
9. tester une commande coupée en plusieurs morceaux ;
10. tuer un client brutalement ;
11. vérifier Valgrind ;
12. tester `KICK`, `INVITE`, `TOPIC` et `MODE`.

## 13. Limitations

Les points suivants doivent encore être confirmés pendant une vraie évaluation :

- test avec le client IRC de référence choisi par l'équipe ;
- test `Ctrl+Z` sur un client puis flood d'un canal ;
- vérification dans un clone propre du dépôt officiel ;
- vérification Norminette si elle est demandée par l'évaluation locale.

## 14. Conclusion

Pour la partie obligatoire, le projet semble prêt.

Estimation pour la section opérateur : `5/5`, si les mêmes tests passent pendant
la soutenance.
