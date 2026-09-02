*This project has been created as part of the 42 curriculum by fepopadi, diahussa, dleite-b.*

# ft_irc

## Description

`ft_irc` is a non-blocking IRC server written in C++98. It accepts multiple TCP clients simultaneously and implements the mandatory IRC registration, private messaging, channels, channel operators, topics, invitations, kicks, and channel modes.

The project focuses on network programming, the IRC text protocol, event-driven I/O, partial TCP reads and writes, and consistent shared state between clients and channels.

The reference client used during development and manual testing is **HexChat**.

## Features

- Multiple simultaneous IPv4 TCP clients.
- One central `poll()` call for accepting, reading, and writing.
- Non-blocking listening and client sockets.
- Per-client input reconstruction and output queues.
- Partial read, multiple-command packet, and partial write handling.
- IRC registration with `PASS`, `NICK`, and `USER`.
- Connection commands: `PING`, `PONG`, `QUIT`, and nickname changes.
- Channel commands: `JOIN`, `PART`, `PRIVMSG`, `NOTICE`, `NAMES`, and `WHO`.
- Operator commands: `KICK`, `INVITE`, `TOPIC`, and `MODE`.
- Mandatory channel modes:
  - `i`: invite-only;
  - `t`: operator-only topic changes;
  - `k`: channel key;
  - `o`: channel operator privilege;
  - `l`: user limit.
- Minimal `CAP` negotiation for IRC client compatibility.
- Automatic cleanup of disconnected clients and empty channels.

## Instructions

### Requirements

- A Unix-like operating system.
- A C++ compiler with C++98 support.
- `make`.
- An IRC client such as HexChat, or `nc` for low-level testing.

### Compilation

```bash
make
```

The build creates the executable `ircserv`.

Available Makefile targets:

```bash
make        # build the server
make clean  # remove object files
make fclean # remove object files and ircserv
make re     # rebuild from scratch
```

The project is compiled with:

```text
-Wall -Wextra -Werror -std=c++98 -pedantic
```

### Running the server

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 secret
```

The port must be between `1` and `65535`, and the password must not be empty.

Stop the server with `Ctrl+C`.

## Connecting with HexChat

Create a custom HexChat network with the following settings:

```text
Server: 127.0.0.1/6667
SSL: disabled
Login method: Server Password
Password: secret
Nickname: alice
User name: alice
Real name: Alice
```

Use the same port and password passed to `ircserv`.

After connecting, try:

```text
/join #general
/topic #general A test channel
/mode #general +it
/invite bob #general
/mode #general +o bob
/kick #general bob Test kick
```

Plain text entered in a channel is sent as `PRIVMSG` by HexChat.

## Connecting with Netcat

```bash
nc 127.0.0.1 6667
```

Register manually:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Doe
```

The server completes registration with numeric `001`.

Example channel session:

```text
JOIN #general
PRIVMSG #general :Hello everyone
PART #general :Leaving
QUIT :Goodbye
```

## Architecture

```text
main
└── listening socket
    └── central poll loop
        ├── accept new clients
        ├── read client data on POLLIN
        │   └── reconstruct lines
        │       └── parse and dispatch IRC commands
        ├── write queued data on POLLOUT
        └── clean up errors and disconnections
```

Core components:

- `Client`: identity, registration state, input buffer, output queue, and channel memberships.
- `ClientDataBase`: pending and registered clients, file descriptor lookup, and nickname lookup.
- `IRCCommand`: IRC line parser.
- `Channel`: members, operators, invitations, topic, key, limit, and modes.
- `Server`: global password, client database, and channel collection.

More detailed documentation is available in:

- `MANUAL_DE_USO.md` — usage guide in Portuguese;
- `FLUXO_DO_PROGRAMA.md` — program flow and architecture in Portuguese;
- `AUDITORIA_FT_IRC.md` — final compliance and test status.

## Testing

Recommended checks include:

- fragmented commands and multiple commands in one TCP packet;
- invalid and valid registration sequences;
- duplicate and changed nicknames;
- multiple clients and multiple channels;
- direct and channel messages;
- every mandatory channel mode with positive and negative cases;
- abrupt disconnections and `QUIT`;
- slow clients and partial output;
- connection and full command use through HexChat.

Example fragmented-input test:

```bash
printf 'PASS secret\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n' \
  | nc -w 1 127.0.0.1 6667
```

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [IRCv3 message format](https://modern.ircdocs.horse/)
- [Linux `poll(2)` manual](https://man7.org/linux/man-pages/man2/poll.2.html)
- [Linux `socket(2)` manual](https://man7.org/linux/man-pages/man2/socket.2.html)
- [Linux `recv(2)` manual](https://man7.org/linux/man-pages/man2/recv.2.html)
- [Linux `send(2)` manual](https://man7.org/linux/man-pages/man2/send.2.html)
- [HexChat documentation](https://hexchat.readthedocs.io/en/latest/)

## Use of AI

AI was used as a review and productivity tool for:

- comparing the implementation with the project subject;
- identifying missing protocol and non-blocking I/O requirements;
- proposing test cases for fragmented TCP input, output queues, registration, channels, and modes;
- reviewing parser and state-management edge cases;
- helping structure the project documentation.

All generated suggestions were reviewed, compiled, tested, and adapted to the project. The final implementation remains the responsibility of the project author and should be understood well enough to explain and modify during peer evaluation.

## Bonus

The optional file-transfer and bot features are not implemented. The project prioritizes correctness and stability of the mandatory server.
