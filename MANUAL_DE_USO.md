# Manuel d'utilisation de `ft_irc`

Ce manuel explique comment compiler, lancer et tester le serveur.

## 1. Compiler

Depuis le dossier qui contient le `Makefile`, lancer :

```bash
make
```

Si tout se passe bien, un exécutable est créé :

```text
ircserv
```

Autres commandes utiles :

```bash
make clean
make fclean
make re
```

## 2. Lancer le serveur

Syntaxe :

```bash
./ircserv <port> <mot_de_passe>
```

Exemple :

```bash
./ircserv 6667 secret
```

Ici :

- `6667` est le port TCP ;
- `secret` est le mot de passe du serveur.

Le terminal reste occupé tant que le serveur tourne.

Pour arrêter le serveur :

```text
Ctrl+C
```

## 3. Se connecter avec `nc`

Dans un autre terminal :

```bash
nc 127.0.0.1 6667
```

Le port doit être le même que celui utilisé au lancement du serveur.

## 4. S'enregistrer

Après la connexion avec `nc`, envoyer :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
```

Si le mot de passe est correct, le serveur répond avec un message `001`.

Exemple :

```text
:ircserv 001 alice :Welcome to the IRC network alice!alice@localhost
```

`001` signifie que le client est enregistré.

## 5. Connecter deux clients

Terminal 1 :

```bash
./ircserv 6667 secret
```

Terminal 2 :

```bash
nc 127.0.0.1 6667
```

Puis :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
```

Terminal 3 :

```bash
nc 127.0.0.1 6667
```

Puis :

```text
PASS secret
NICK bob
USER bob 0 * :Bob
```

Les deux clients sont maintenant connectés.

## 6. Rejoindre un canal

Dans les deux clients :

```text
JOIN #general
```

Le premier client qui rejoint un canal devient opérateur du canal. Dans la liste
des noms, il apparaît avec `@`.

Exemple :

```text
:ircserv 353 alice = #general :@alice bob
```

## 7. Envoyer un message dans un canal

Depuis `alice` :

```text
PRIVMSG #general :salut bob
```

`bob` doit recevoir :

```text
:alice!alice@localhost PRIVMSG #general :salut bob
```

Le serveur envoie le message aux autres membres du canal.

## 8. Envoyer un message privé

Depuis `bob` :

```text
PRIVMSG alice :salut alice
```

`alice` doit recevoir :

```text
:bob!bob@localhost PRIVMSG alice :salut alice
```

## 9. Quitter

Pour quitter proprement :

```text
QUIT :bye
```

Le serveur ferme ensuite la connexion du client.

## 10. Commandes disponibles

Commandes principales :

| Commande | Utilisation |
|---|---|
| `PASS` | envoyer le mot de passe |
| `NICK` | choisir ou changer de nickname |
| `USER` | envoyer les informations utilisateur |
| `PING` | tester la connexion |
| `QUIT` | quitter le serveur |
| `JOIN` | entrer dans un canal |
| `PART` | quitter un canal |
| `PRIVMSG` | envoyer un message |

Commandes de canal obligatoires :

| Commande | Utilisation |
|---|---|
| `KICK` | retirer un membre du canal |
| `INVITE` | inviter un utilisateur |
| `TOPIC` | lire ou modifier le sujet du canal |
| `MODE` | modifier les modes du canal |

Commandes de compatibilité :

| Commande | Utilisation |
|---|---|
| `CAP` | négociation avec certains clients IRC |
| `NAMES` | liste des membres d'un canal |
| `WHO` | informations sur les membres |
| `NOTICE` | message sans réponse d'erreur |

## 11. Exemples de commandes opérateur

Créer un canal :

```text
JOIN #test
```

Le premier utilisateur devient opérateur.

Activer le mode invitation seulement :

```text
MODE #test +i
```

Désactiver le mode invitation seulement :

```text
MODE #test -i
```

Inviter un utilisateur :

```text
INVITE bob #test
```

Définir une clé de canal :

```text
MODE #test +k secretkey
```

Rejoindre un canal avec clé :

```text
JOIN #test secretkey
```

Retirer la clé :

```text
MODE #test -k
```

Limiter le nombre d'utilisateurs :

```text
MODE #test +l 3
```

Retirer la limite :

```text
MODE #test -l
```

Donner les droits opérateur :

```text
MODE #test +o bob
```

Retirer les droits opérateur :

```text
MODE #test -o bob
```

Protéger le topic :

```text
MODE #test +t
```

Changer le topic :

```text
TOPIC #test :Sujet du canal
```

Expulser un utilisateur :

```text
KICK #test bob :raison
```

## 12. Tester une commande partielle

Avec `nc`, il est possible d'envoyer une commande en plusieurs morceaux.

Exemple :

```text
PASS sec
```

Puis finir ensuite :

```text
ret
```

Le serveur doit attendre la fin de la ligne avant de traiter la commande.

Autre exemple :

```text
PRIV
```

Puis :

```text
MSG #general :message coupe
```

Le serveur doit reconstruire :

```text
PRIVMSG #general :message coupe
```

## 13. Tester avec un client IRC

On peut aussi utiliser un vrai client IRC, par exemple HexChat, WeeChat ou irssi.

Paramètres :

```text
serveur: 127.0.0.1
port: 6667
mot de passe: secret
nickname: alice
username: alice
```

Si le client demande des capacités avec `CAP`, le serveur répond de façon simple
pour permettre la connexion.

## 14. Tester Valgrind

Commande :

```bash
valgrind --leak-check=full --track-fds=yes ./ircserv 6667 secret
```

Dans un autre terminal, connecter quelques clients, faire des `JOIN`,
`PRIVMSG`, puis quitter.

À la fin, arrêter le serveur avec `Ctrl+C`.

Résultat attendu :

```text
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors
```

## 15. Séquence rapide pour la défense

Dans le serveur :

```bash
make re
./ircserv 6667 secret
```

Client 1 :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #test
```

Client 2 :

```text
PASS secret
NICK bob
USER bob 0 * :Bob
JOIN #test
PRIVMSG #test :bonjour
```

Client 1 :

```text
MODE #test +o bob
TOPIC #test :demo
KICK #test bob :test
```

Cette séquence montre :

- l'enregistrement ;
- plusieurs clients ;
- un canal ;
- un message de canal ;
- les droits opérateur ;
- `TOPIC` ;
- `KICK`.
