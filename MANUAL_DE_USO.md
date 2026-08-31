# Manual de inicialização e uso do `ft_irc`

## 1. Estado atual do projeto

O servidor já pode ser compilado, iniciado e receber conexões TCP de vários clientes. O registro IRC com `PASS`, `NICK` e `USER` está implementado.

Neste momento, os seguintes comandos funcionam:

- `PASS` — envia a senha do servidor;
- `NICK` — escolhe o nickname inicial;
- `USER` — envia username e nome real.
- `PING` — verifica a conexão e recebe `PONG`;
- `PONG` — confirma uma verificação enviada pelo servidor;
- `NICK` — também altera o nickname depois do registro;
- `QUIT` — encerra a conexão com motivo opcional.
- `JOIN` — entra em um ou vários canais;
- `PART` — sai de um ou vários canais;
- `PRIVMSG` — envia mensagens privadas ou para canais.

Os comandos obrigatórios `KICK`, `INVITE`, `TOPIC` e `MODE` também estão implementados. A sintaxe antiga com `send`, `create` e `enter` não faz mais parte do fluxo atual.

Um cliente pode estar associado a vários canais simultaneamente.

## 2. Abrir o diretório do projeto

No terminal, entre na pasta que contém o Makefile:

```bash
cd IA_IRC/AI_IRC
```

Para confirmar que está no diretório correto:

```bash
pwd
ls
```

Entre os arquivos mostrados devem estar `Makefile`, `main.cpp` e `boucle_principale.cpp`.

## 3. Compilar o servidor

Execute:

```bash
make
```

Se a compilação terminar corretamente, será criado o executável:

```text
ircserv
```

Para apagar somente os arquivos objeto (`.o`):

```bash
make clean
```

Para apagar os objetos e o executável:

```bash
make fclean
```

Para apagar e recompilar tudo:

```bash
make re
```

## 4. Iniciar o servidor

A sintaxe é:

```bash
./ircserv <porta> <senha>
```

Exemplo:

```bash
./ircserv 6667 secret
```

Nesse exemplo:

- `6667` é a porta TCP;
- `secret` é a senha que cada cliente deve enviar com `PASS`.

O terminal ficará ocupado enquanto o servidor estiver funcionando. Isso é normal: o processo está aguardando conexões e mensagens.

### Escolha da porta

A porta deve ser um número entre `1` e `65535`. Para testes, é recomendável usar uma porta alta, como:

```bash
./ircserv 6667 secret
./ircserv 6668 secret
./ircserv 8080 secret
```

Se a porta já estiver ocupada, escolha outra.

## 5. Parar o servidor

No terminal onde o servidor está executando, pressione:

```text
Ctrl+C
```

O atalho envia `SIGINT`; o servidor encerra o loop e fecha os sockets.

Também é possível enviar `SIGTERM` a partir de outro terminal:

```bash
pkill -TERM ircserv
```

Use `pkill` somente quando tiver certeza de que deseja parar todos os processos chamados `ircserv` pertencentes ao seu usuário.

## 6. Conectar um cliente com `nc`

Mantenha o servidor aberto no primeiro terminal. Abra um segundo terminal e execute:

```bash
nc 127.0.0.1 6667
```

Significado dos argumentos:

- `127.0.0.1` significa que o cliente e o servidor estão no mesmo computador;
- `6667` deve ser a mesma porta usada para iniciar o servidor.

Se o comando `nc` não existir, verifique se o Netcat está instalado no sistema. Em algumas versões, o programa pode se chamar `netcat`.

## 7. Registrar-se no servidor

Depois de conectar com `nc`, envie estas três linhas, pressionando Enter ao final de cada uma:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Doe
```

Troque `secret` pela senha usada ao iniciar o servidor.

### Explicação de cada linha

```text
PASS secret
```

Envia a senha global do servidor.

```text
NICK alice
```

Define `alice` como nickname. O nickname deve:

- ter no máximo 30 caracteres;
- começar com uma letra ou caractere especial autorizado;
- não conter espaços;
- não estar sendo usado por outro cliente conectado.

```text
USER alice 0 * :Alice Doe
```

Define:

- `alice` como username;
- `0` e `*` como parâmetros tradicionais do comando IRC;
- `Alice Doe` como nome real, depois de `:`.

Quando tudo estiver correto, o servidor responde aproximadamente:

```text
:ircserv 001 alice :Welcome to the IRC network alice!alice@localhost
```

O código `001` confirma que o registro foi concluído.

## 8. Ordem dos comandos de registro

Os comandos podem chegar em ordens diferentes. Por exemplo, isto também funciona:

```text
NICK alice
USER alice 0 * :Alice Doe
PASS secret
```

O servidor espera até possuir senha válida, nickname e dados de usuário antes de enviar `001`.

Por clareza e compatibilidade com clientes IRC, recomenda-se usar a ordem:

```text
PASS
NICK
USER
```

## 9. Conectar dois clientes

Primeiro terminal — servidor:

```bash
./ircserv 6667 secret
```

Segundo terminal — primeiro cliente:

```bash
nc 127.0.0.1 6667
```

Registro do primeiro cliente:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Doe
```

Terceiro terminal — segundo cliente:

```bash
nc 127.0.0.1 6667
```

Registro do segundo cliente:

```text
PASS secret
NICK bob
USER bob 0 * :Bob Smith
```

Os dois clientes ficam conectados simultaneamente e já podem conversar usando `PRIVMSG`.

### Entrar em canais

Em cada cliente:

```text
JOIN #geral
```

Para entrar em vários canais de uma vez:

```text
JOIN #geral,#programacao,#testes
```

O primeiro cliente de um canal recebe privilégio de operador, indicado por `@` na resposta `353`.

### Enviar mensagem privada

Alice envia para Bob:

```text
PRIVMSG bob :Olá Bob!
```

Bob recebe:

```text
:alice!alice@localhost PRIVMSG bob :Olá Bob!
```

### Enviar mensagem para um canal

```text
PRIVMSG #geral :Olá a todos!
```

A mensagem é entregue aos outros membros do canal, mas não retorna ao remetente.

### Sair de um canal

```text
PART #geral :Até logo
```

Para sair de vários canais:

```text
PART #geral,#programacao :Até logo
```

Para sair de todos os canais:

```text
JOIN 0
```

### Consultar e alterar o tópico

Consultar:

```text
TOPIC #geral
```

Alterar ou limpar:

```text
TOPIC #geral :Novo tópico
TOPIC #geral :
```

Com o modo `+t`, somente operadores podem alterar o tópico.

### Convidar um usuário

Somente um operador pode executar:

```text
INVITE bob #geral
```

O convite permite que Bob entre quando o canal estiver em modo `+i`:

```text
JOIN #geral
```

### Expulsar um usuário

```text
KICK #geral bob :Motivo da expulsão
```

Somente operadores do canal podem usar `KICK`.

### Consultar modos

```text
MODE #geral
```

### Modos disponíveis

Canal somente por convite:

```text
MODE #geral +i
MODE #geral -i
```

Tópico restrito a operadores:

```text
MODE #geral +t
MODE #geral -t
```

Definir ou remover senha do canal:

```text
MODE #geral +k senhaCanal
MODE #geral -k
```

Para entrar em um canal protegido:

```text
JOIN #geral senhaCanal
```

Promover ou remover operador:

```text
MODE #geral +o bob
MODE #geral -o bob
```

Definir ou remover limite de usuários:

```text
MODE #geral +l 10
MODE #geral -l
```

Modos podem ser combinados quando os parâmetros aparecem na mesma ordem:

```text
MODE #geral +it
MODE #geral +kl senhaCanal 10
```

## 10. Atalhos de teclado úteis

### No terminal do servidor

- `Ctrl+C` — para o servidor de forma controlada.

### No `nc`

- `Enter` — termina e envia uma linha;
- `Ctrl+D` — envia o conteúdo pendente/indica fim da entrada, dependendo da versão do Netcat e do estado da linha;
- `Ctrl+C` — encerra o cliente `nc` imediatamente;
- `Ctrl+L` — limpa visualmente o terminal sem apagar o histórico da sessão.

No exemplo de fragmentação do enunciado, `Ctrl+D` pode ser usado para enviar partes de uma linha antes do Enter. O servidor mantém os fragmentos no buffer até receber `\n`.

## 11. Testar comandos fragmentados

Conecte-se com `nc` e escreva parte de um comando sem pressionar Enter:

```text
PA
```

Pressione `Ctrl+D` uma vez para tentar enviar esse fragmento. Em seguida, digite:

```text
SS secret
```

Pressione Enter. O servidor deve reconstruir:

```text
PASS secret
```

O comportamento exato de `Ctrl+D` pode variar entre implementações do Netcat. O teste automatizado com `printf` é mais previsível.

## 12. Enviar vários comandos de uma vez

É possível enviar todo o registro em uma única chamada:

```bash
printf 'PASS secret\r\nNICK alice\r\nUSER alice 0 * :Alice Doe\r\n' | nc -w 1 127.0.0.1 6667
```

O servidor separa todas as linhas completas e processa cada comando na ordem recebida.

## 13. Manter e encerrar a conexão

### Testar com `PING`

Depois de conectar, envie um identificador qualquer:

```text
PING 12345
```

Resposta esperada:

```text
:ircserv PONG ircserv :12345
```

Se `PING` for enviado sem parâmetro, o servidor responde com o erro `409`.

### Enviar `PONG`

Um cliente pode responder a uma verificação do servidor com:

```text
PONG 12345
```

O servidor aceita `PONG` silenciosamente.

### Alterar o nickname

Depois de receber `001`, escolha um novo nickname:

```text
NICK bob
```

O cliente e os membros dos canais associados recebem um evento semelhante a:

```text
:alice!alice@localhost NICK :bob
```

As mesmas regras de sintaxe e unicidade do registro inicial continuam válidas.

### Sair com `QUIT`

Para encerrar informando um motivo:

```text
QUIT :Até logo
```

Sem motivo explícito:

```text
QUIT
```

O servidor entrega as respostas já pendentes, envia uma linha `ERROR`, remove o cliente dos canais, libera o nickname e fecha a conexão.

## 14. Respostas de erro

As respostas IRC começam com `:ircserv`, seguidas por um código numérico.

### Senha incorreta — `464`

Entrada:

```text
PASS wrong
```

Resposta:

```text
:ircserv 464 * :Password incorrect
```

É possível tentar `PASS` novamente com a senha correta.

### Nickname ausente — `431`

Entrada:

```text
NICK
```

Resposta esperada:

```text
:ircserv 431 * :No nickname given
```

### Nickname inválido — `432`

Entrada:

```text
NICK 123alice
```

Resposta:

```text
:ircserv 432 * 123alice :Erroneous nickname
```

### Nickname em uso — `433`

Se outro cliente conectado já estiver usando `alice`:

```text
NICK alice
```

Resposta:

```text
:ircserv 433 * alice :Nickname is already in use
```

### Parâmetros insuficientes — `461`

Exemplo:

```text
USER alice
```

Resposta:

```text
:ircserv 461 * USER :Not enough parameters
```

### Cliente ainda não registrado — `451`

Se um comando diferente de `PASS`, `NICK` ou `USER` for enviado antes da conclusão do registro:

```text
PRIVMSG bob :hello
```

Resposta:

```text
:ircserv 451 * PRIVMSG :You have not registered
```

### Comando desconhecido — `421`

Depois do registro:

```text
QUALQUERCOISA
```

Resposta:

```text
:ircserv 421 alice QUALQUERCOISA :Unknown command
```

### Tentativa de registrar novamente — `462`

Depois de receber `001`, tentar executar `PASS` ou `USER` novamente resulta em `462`.

## 15. Problemas comuns

### `Usage: ./ircserv <port> <password>`

O programa foi iniciado sem os dois argumentos obrigatórios. Use:

```bash
./ircserv 6667 secret
```

### `Error: port must be a number between 1 and 65535`

A porta está vazia, contém letras ou está fora da faixa permitida.

Incorreto:

```bash
./ircserv abc secret
./ircserv 70000 secret
```

Correto:

```bash
./ircserv 6667 secret
```

### `Error: password must not be empty`

A senha não pode ser uma string vazia.

### `Error: could not bind to port`

Normalmente significa que a porta já está em uso ou não está disponível. Pare o servidor anterior com `Ctrl+C` ou escolha outra porta:

```bash
./ircserv 6668 secret
```

### O `nc` fecha imediatamente

Verifique:

- se o servidor continua executando;
- se o endereço é `127.0.0.1`;
- se a porta é a mesma nos dois terminais;
- se outro programa não está ocupando a porta.

### Nenhuma resposta aparece depois de `PASS`

Isso pode ser normal. O servidor envia `001` somente depois de receber os três comandos válidos:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Doe
```

## 16. Teste rápido completo

Terminal 1:

```bash
cd IA_IRC/AI_IRC
make
./ircserv 6667 secret
```

Terminal 2:

```bash
nc 127.0.0.1 6667
```

Digite no terminal 2:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Doe
```

Resultado esperado:

```text
:ircserv 001 alice :Welcome to the IRC network alice!alice@localhost
```

Para terminar:

1. pressione `Ctrl+C` no terminal do `nc`;
2. pressione `Ctrl+C` no terminal do servidor.

## 17. Estado final e próximos passos

Todos os comandos obrigatórios e os auxiliares `CAP`, `NAMES`, `WHO` e `NOTICE` estão implementados. O HexChat foi escolhido e validado como cliente de referência.

Antes da entrega:

1. confirme que o login `dleite-b` aparece na primeira linha do README;
2. coloque o projeto no repositório Git oficial da 42;
3. faça commit e push nesse repositório;
4. clone o repositório em outro diretório e execute `make`;
5. repita os testes principais com dois clientes HexChat.
