# `ft_irc` — fiche de défense suivant l'eval sheet

Utiliser **trois terminaux** :

- **T1** : serveur ;
- **T2** : `nc` avec `alice` ;
- **T3** : `nc` avec `bob` ou les tests spéciaux.

Les valeurs utilisées ci-dessous sont : port `6667`, mot de passe `secret`.

---

## 1. Basic checks — points éliminatoires

### Compiler et lancer

```bash
make fclean
make
./ircserv 6667 secret
```

À dire : « Le projet est en C++98, compilé avec `-Wall -Wextra -Werror`. Le binaire s'appelle `ircserv` et prend `<port> <password>`. »

### Vérifier `poll`, `fcntl` et `errno`

Dans un autre terminal, à la racine du projet :

```bash
grep -RIn --include='*.cpp' 'poll[[:space:]]*(' .
grep -RIn --include='*.cpp' 'fcntl[[:space:]]*(' .
grep -RIn --include='*.cpp' 'errno\|EAGAIN\|EWOULDBLOCK' .
```

Résultat attendu :

- **un seul `poll()`**, dans `boucle_principale.cpp` ;
- seulement **deux `fcntl(fd, F_SETFL, O_NONBLOCK)`** : socket serveur et nouveau client ;
- aucune action déclenchée avec `errno`, `EAGAIN` ou `EWOULDBLOCK`.

À dire : « `accept()` est appelé après `POLLIN` du socket serveur, `recv()` après `POLLIN` d'un client et `send()` après `POLLOUT`. Les réponses sont placées dans un buffer de sortie par client. »

---

## 2. Networking

### Vérifier l'écoute sur toutes les interfaces

```bash
lsof -nP -iTCP:6667 -sTCP:LISTEN
```

Le résultat doit afficher `*:6667`. Dans le code :

```bash
grep -n 'INADDR_ANY' create_listening_socket.cpp
```

### Connecter deux clients `nc`

Dans **T2** :

```bash
nc 127.0.0.1 6667
```

Puis :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #test
```

Dans **T3**, refaire `nc 127.0.0.1 6667`, puis :

```text
PASS secret
NICK bob
USER bob 0 * :Bob
JOIN #test
PRIVMSG #test :bonjour tout le monde
```

À montrer : réponse numérique `001`, les deux clients dans `#test` et `alice` reçoit le message de `bob`.

### Client IRC de référence

Réponse : **HexChat**.

Configuration HexChat :

```text
Server       127.0.0.1/6667
SSL          désactivé
Login method Server Password
Password     secret
```

Se connecter pendant que les deux `nc` fonctionnent, puis taper :

```text
/join #test
bonjour depuis HexChat
```

À montrer : HexChat et les deux `nc` fonctionnent en même temps et reçoivent les messages du canal.

---

## 3. Networking specials

### Commande envoyée en deux morceaux

Dans un nouveau terminal :

```bash
{ printf 'PASS secret\r\nNICK par'; sleep 10; printf 'tial\r\nUSER partial 0 * :Partial\r\n'; sleep 5; } | nc 127.0.0.1 6667
```

Pendant les 10 secondes, depuis le `nc` de `alice` :

```text
PING :42
```

Résultat attendu : aucune inscription du client partiel avant le second morceau ; `alice` reçoit immédiatement `PONG ... :42` ; après 10 secondes, le client `partial` reçoit `001`.

À dire : « Chaque client possède son propre buffer d'entrée. Une ligne est traitée seulement quand `\n` est reçu. »

### Tuer brutalement un client

Dans le terminal de `bob`, faire `Ctrl+C`. Dans celui de `alice` :

```text
PING :apres-kill
```

Résultat attendu : `PONG`; puis ouvrir un nouveau terminal et vérifier qu'une nouvelle commande `nc 127.0.0.1 6667` se connecte.

### Tuer un client après une demi-commande

```bash
{ printf 'PASS secret\r\nNICK half\r\nUSER half 0 * :Half\r\n'; sleep 1; printf 'PRIVMSG #test :commande incomplète'; sleep 999; } | nc 127.0.0.1 6667
```

Faire `Ctrl+C` pendant le dernier `sleep`, puis dans le `nc` de `alice` :

```text
PING :apres-demi-commande
```

Résultat attendu : `PONG` et une nouvelle connexion toujours possible.

### Suspendre un client et flooder

1. Garder `alice` connectée dans `#test`, puis faire `Ctrl+Z` dans son terminal.
2. Dans un autre terminal, lancer :

```bash
{ printf 'PASS secret\r\nNICK flooder\r\nUSER flooder 0 * :Flooder\r\nJOIN #test\r\n'; sleep 1; i=1; while [ "$i" -le 1000 ]; do printf 'PRIVMSG #test :message-%04d-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n' "$i"; i=$((i + 1)); done; printf 'PRIVMSG #test :FIN-FLOOD\r\n'; sleep 3; } | nc 127.0.0.1 6667
```

3. Vérifier avec un autre client que le serveur répond encore à `PING :flood`.
4. Revenir au terminal suspendu et taper :

```bash
fg
```

Résultat attendu : le serveur n'a pas bloqué et `alice` reçoit les messages jusqu'à `FIN-FLOOD`.

---

## 4. Client Commands basic

Avec `nc` puis HexChat, montrer :

```text
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #test
```

Tester les variantes de `PRIVMSG` :

```text
PRIVMSG bob :message privé
PRIVMSG #test :message canal
PRIVMSG bob,#test :plusieurs destinations
PRIVMSG inconnu :test
PRIVMSG
PRIVMSG bob
```

Attendu : messages privé/canal transmis ; utilisateur inconnu = `401` ; destinataire absent = `411` ; texte absent = `412`.

---

## 5. Client Commands channel operator

Préparation : `alice` rejoint `#test` en premier et devient opératrice ; `bob` rejoint ensuite ; `charlie` est enregistré mais reste hors du canal.

### Vérifier qu'un utilisateur normal est refusé

Depuis `bob` :

```text
KICK #test alice :interdit
INVITE charlie #test
MODE #test +i
```

Attendu : erreur `482` — `You're not channel operator`.

### `KICK`

Depuis `alice` :

```text
KICK #test bob :test kick
```

Puis `bob` refait `JOIN #test`.

### Mode `i` et `INVITE`

Depuis `alice` :

```text
MODE #test +i
```

Depuis `charlie`, `JOIN #test` doit donner `473`. Ensuite, depuis `alice` :

```text
INVITE charlie #test
```

Puis depuis `charlie` :

```text
JOIN #test
```

Enfin, depuis `alice` :

```text
KICK #test charlie :fin test invite
MODE #test -i
```

### Mode `t` et `TOPIC`

Depuis `alice` :

```text
MODE #test +t
```

Depuis `bob` — doit recevoir `482` :

```text
TOPIC #test :topic interdit
```

Depuis `alice` — doit réussir :

```text
TOPIC #test :nouveau sujet
TOPIC #test
MODE #test -t
```

### Mode `k`

Depuis `alice` :

```text
MODE #test +k cle42
```

Depuis `charlie` — doit recevoir `475` :

```text
JOIN #test mauvaise-cle
```

Depuis `charlie` — doit réussir :

```text
JOIN #test cle42
```

Depuis `alice` :

```text
KICK #test charlie :fin test key
MODE #test -k
```

### Mode `o`

Depuis `alice` :

```text
MODE #test +o bob
```

Depuis `bob`, maintenant opérateur — doit réussir :

```text
MODE #test +i
```

Depuis `alice` :

```text
MODE #test -i
MODE #test -o bob
```

Depuis `bob` — doit de nouveau recevoir `482` :

```text
MODE #test +i
```

### Mode `l`

Avec seulement `alice` et `bob` dans le canal :

Depuis `alice` :

```text
MODE #test +l 2
```

Depuis `charlie` — doit recevoir `471` :

```text
JOIN #test
```

Depuis `alice` :

```text
MODE #test -l
```

Depuis `charlie` — doit maintenant réussir :

```text
JOIN #test
```

---

## 6. Fuites mémoire et crash

Lancer le serveur dès le début avec un outil mémoire à la place de la commande normale.

Linux :

```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 secret
```

macOS :

```bash
leaks --atExit -- ./ircserv 6667 secret
```

Faire les connexions, `JOIN`, `PRIVMSG`, déconnexions et flood, puis arrêter le serveur avec `Ctrl+C`. Attendu : aucune fuite et aucune erreur mémoire.

---

## 7. Bonus

Réponse honnête : **pas de transfert de fichier et pas de bot**. Les bonus sont ignorés si la partie obligatoire n'est pas parfaite.

## Rappels éliminatoires

- Ne modifier aucun fichier pendant l'évaluation.
- Mauvais `poll()`/`fcntl()`, compilation impossible, crash, blocage ou fuite mémoire peuvent arrêter l'évaluation avec **0**.
- Si une commande échoue, expliquer calmement ce qui se passe ; ne pas improviser une modification du code.
