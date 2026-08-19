# Auditoria do projeto `ft_irc`

## Resultado geral

**Estado atual: o projeto ainda não cumpre a parte obrigatória e não está pronto para avaliação.**

O código compila com `-Wall -Wextra -Werror -std=c++98 -pedantic`, e um segundo `make` não relinka. Porém, o programa implementa um protocolo de chat próprio, não o protocolo IRC esperado pelo enunciado. Um cliente IRC de referência não conseguirá completar o registro nem executar os comandos obrigatórios.

Esta auditoria foi feita comparando `en.subject.pdf` (versão 11.0) com os fontes atuais e executando `make fclean`, `make` e um segundo `make`.

## Progresso de implementação

Os itens 1 a 9 da ordem recomendada foram implementados em 19/08/2026:

- o Makefile agora gera `ircserv`, os argumentos e o porto são validados e a inicialização/terminação do socket possui tratamento de falhas;
- `Server` armazena a senha global e `Client` possui estado de registro IRC, username/realname, buffer de entrada e fila de saída;
- `IRCCommand` interpreta prefixo, comando, parâmetros comuns e parâmetro final; a leitura extrai todas as linhas completas e preserva fragmentos.
- todas as respostas são enfileiradas no `Client`; `POLLOUT` é ativado somente enquanto há dados, um único `send()` é tentado por evento e somente os bytes efetivamente enviados são removidos;
- clientes lentos têm fila de saída limitada a 1 MiB e são desconectados em caso de estouro.
- o registro IRC aceita `PASS`, `NICK` e `USER` em ordens diferentes, valida a senha global e a sintaxe/unicidade do nickname e conclui com o numeric `001`;
- foram adicionados os erros `421`, `431`, `432`, `433`, `451`, `461`, `462` e `464`, todos terminados em `\r\n`.
- `PING` responde com `PONG`, inclusive durante o registro, e a ausência de origem retorna `409`;
- `PONG` é aceito sem resposta, `QUIT` propaga o motivo e fecha somente depois de esvaziar a fila, e `NICK` permite alteração depois do registro;
- alterações de nickname atualizam o banco e as associações atuais de canais; desconexões removem o cliente dos canais e liberam o nickname.
- cada cliente mantém um conjunto de canais e pode pertencer simultaneamente a vários deles; `_active_channel` e `_active_members` foram removidos;
- entrada/saída atualiza os dois lados da associação, notificações em canais compartilhados deduplicam destinatários e canais vazios são removidos;
- o handler do protocolo de chat antigo foi retirado da compilação e removido, deixando uma única base para os próximos comandos IRC.
- `JOIN` aceita um ou vários canais, cria canais automaticamente, promove o primeiro membro a operador e envia `331`, `353` e `366`;
- `PART` aceita um ou vários canais e motivo opcional; `JOIN 0` permite sair de todos;
- `PRIVMSG` entrega mensagens diretas e de canal, aceita múltiplos destinatários e não devolve ao remetente mensagens enviadas ao canal;
- canais são localizados sem diferenciar maiúsculas/minúsculas e os erros `401`, `403`, `404`, `411`, `412` e `442` foram adicionados.
- `KICK`, `INVITE`, consulta/alteração de `TOPIC` e consulta/alteração de `MODE` estão implementados com verificação de operador;
- os modos obrigatórios `i`, `t`, `k`, `o` e `l` funcionam com `+` e `-`, inclusive em combinações;
- `JOIN` respeita convite, chave e limite; foram adicionados os numerics `324`, `341`, `441`, `443`, `467`, `471`, `472`, `473`, `475` e `482`.

Os tópicos críticos abaixo descrevem a auditoria original. Os itens já corrigidos devem ser lidos em conjunto com esta seção; o passo 10 e a validação final continuam pendentes.

## Pontos que já existem

- Há um loop central baseado em um único `poll()`.
- Novos sockets de clientes são colocados em modo não bloqueante.
- Há armazenamento parcial de entrada por cliente até aparecer `\n`.
- Há estruturas iniciais para clientes, canais, membros, convidados e operadores.
- Existem ideias preliminares equivalentes a mensagem privada, criação/entrada em canal, convite, expulsão e promoção de operador.
- O projeto compila com as flags obrigatórias.
- O Makefile possui `all`, `clean`, `fclean` e `re` e não relinka quando nada mudou.

Esses pontos são apenas uma base. As interfaces e respostas atuais não são IRC e precisam ser reformuladas.

## Problemas eliminatórios ou críticos

### 1. Nome do executável incorreto

O enunciado exige `ircserv`, mas o Makefile gera `web` (`Makefile`, variável `NAME`). A execução exigida é:

```sh
./ircserv <port> <password>
```

### 2. A senha passada ao servidor é ignorada

**Resolvido nos itens 2 e 5.** A senha global é armazenada no `Server` e validada por `PASS`.

Em `main.cpp`, `password` é descartada com `(void)password`. O projeto deve validar o comando IRC `PASS` usando a senha fornecida na linha de comando. O sistema atual de criação de contas e senhas individuais não é solicitado pelo projeto e não substitui essa autenticação.

### 3. Não existe protocolo IRC

O servidor envia perguntas em texto livre e espera respostas como nome, senha, `send`, `create`, `enter` e `give admin`. Clientes IRC enviam linhas no formato IRC, por exemplo:

```text
PASS secret\r\n
NICK alice\r\n
USER alice 0 * :Alice Doe\r\n
JOIN #general\r\n
PRIVMSG #general :hello\r\n
```

É necessário implementar um parser IRC (prefixo opcional, comando, parâmetros e parâmetro final iniciado por `:`), aceitar `\r\n` e responder usando mensagens/numerics IRC. As mensagens atuais como `bonjour donne votre nom` tornam incompatível qualquer cliente IRC real.

### 4. Registro obrigatório ausente

**Resolvido nos itens 5 e 6.** O registro inicial e a troca de nickname estão implementados.

Faltam, com sua validação e respostas IRC:

- `PASS` para a senha global do servidor;
- `NICK` para definir e alterar nickname, incluindo unicidade;
- `USER` para definir username/realname;
- controle do estado de registro independentemente da ordem usual de `PASS`, `NICK` e `USER`;
- mensagem de boas-vindas (`001`, e preferencialmente a sequência mínima esperada pelo cliente de referência);
- rejeição de comandos que exigem registro antes de o cliente estar registrado.

### 5. Comandos obrigatórios ausentes

**Resolvido nos itens 8 e 9.** `JOIN`, `PART`, `PRIVMSG` e todos os comandos obrigatórios de operador estão implementados.

Os nomes parecidos existentes são comandos internos próprios e **não contam** como implementação IRC. Faltam:

- `JOIN` — criar/entrar em canais e enviar evento, tópico e lista de nomes;
- `PRIVMSG` — mensagem para nickname e para canal;
- `KICK` — somente operador, com canal, alvo e motivo;
- `INVITE` — somente operador, com nickname e canal;
- `TOPIC` — consultar e alterar tópico, respeitando o modo `t`;
- `MODE` de canal com `+i/-i`, `+t/-t`, `+k/-k`, `+o/-o` e `+l/-l`.

Também são necessários comandos auxiliares normalmente indispensáveis para um cliente real, em especial `PING`/`PONG`, `QUIT` e `PART`. Dependendo do cliente de referência, podem ser necessários `CAP`, `WHO`, `NAMES`, `NOTICE` e tratamento seguro de comandos desconhecidos.

### 6. Modos de canal não existem

**Resolvido no item 9.** O canal armazena tópico, convite obrigatório, proteção do tópico, chave, limite e operadores; `MODE` consulta e altera todos os modos obrigatórios.

`Channel` não armazena:

- estado invite-only (`i`);
- restrição do tópico a operadores (`t`);
- chave/senha (`k`);
- limite de usuários (`l`);
- tópico e autor/data do tópico.

Existe uma coleção de administradores que pode servir de base ao modo `o`, mas ela ainda não é controlada pelo comando IRC `MODE` nem produz as notificações e erros corretos.

### 7. Escritas não são controladas por `poll()`

**Resolvido no item 4.** Não existem mais envios diretos fora do tratamento de `POLLOUT`.

O enunciado determina nota zero se `send`/`recv` for usado sem antes o descritor estar indicado como pronto pelo `poll` (ou equivalente). O programa monitora somente `POLLIN`, mas chama `send()` diretamente em vários locais:

- saudação logo após `accept` (`boucle_principale.cpp`);
- `send_message()` (`handle_client_data/handle_client_data.cpp`);
- broadcast (`classes/channel.cpp`).

É preciso manter uma fila de saída por cliente, habilitar `POLLOUT` quando houver dados pendentes, enviar somente quando `poll()` indicar `POLLOUT` e conservar o restante em caso de envio parcial. Não se deve decidir repetir uma operação com base em `errno == EAGAIN` após `send`/`recv`.

### 8. Mensagens completas e parciais são processadas incorretamente

O buffer parcial é acumulado, mas, ao encontrar qualquer `\n`, todo o conteúdo é tratado como se fosse um único comando e depois apagado. Assim, um pacote contendo:

```text
NICK alice\r\nUSER alice 0 * :Alice\r\n
```

seria interpretado como uma única mensagem. Também se perde uma linha completa seguida pelo começo da próxima. É necessário extrair e processar **todas** as linhas completas, em ordem, preservando o sufixo incompleto para o próximo `recv`.

### 9. Tratamento de desconexão e erros está incorreto

**Resolvido estruturalmente nos itens 1, 6 e 7.** EOF, falhas de leitura, `QUIT` e eventos de erro removem o cliente de todos os canais; canais vazios também são apagados.

Em `handle_client_data.cpp`, qualquer `recv <= 0` é tratado como desconexão. Um erro não equivale a EOF e deve ser tratado separadamente. Além disso:

- não há broadcast de `QUIT` aos canais;
- o cliente não é removido de todos os canais;
- canais vazios não são removidos;
- eventos `POLLHUP`, `POLLERR` e `POLLNVAL` não são tratados;
- não há encerramento limpo do socket de escuta quando o loop termina;
- falha de `poll()` apenas encerra silenciosamente o loop.

### 10. Inicialização do servidor não é robusta

Em `create_listening_socket.cpp` e `boucle_principale.cpp` faltam:

- validação rigorosa do porto (numérico, faixa válida e sem lixo adicional);
- teste da falha de `socket()`;
- `setsockopt(..., SO_REUSEADDR, ...)` para reinício previsível;
- inicialização completa de `sockaddr_in` (por exemplo, zerar a estrutura);
- teste do retorno de `listen()`;
- colocação do socket de escuta em modo não bloqueante;
- teste do retorno de `fcntl()`;
- impedir que `poll()` receba `fd == -1` após falha de criação/bind;
- mensagem de erro e código de saída não zero em falhas;
- validação da quantidade de argumentos com uma mensagem de uso clara em `stderr`.

### 11. Envios parciais são perdidos

**Resolvido no item 4.** O sufixo não enviado permanece na fila para um próximo evento `POLLOUT`, e a fila é limitada a 1 MiB.

O código considera que um único `send()` transmite a mensagem inteira. TCP pode aceitar apenas parte dos bytes. Tanto mensagens individuais quanto broadcasts precisam guardar o trecho não enviado na fila de saída. Também é preciso definir uma política para clientes lentos e limitar buffers para evitar consumo ilimitado de memória.

### 12. Broadcast atual tem semântica errada

**Resolvido nos itens 7 e 8.** Não existe mais “canal ativo”, clientes podem participar de vários canais, destinatários compartilhados são deduplicados e `PRIVMSG` de canal exclui o remetente.

`Channel::broadcast()` envia inclusive ao remetente e usa apenas `_active_members`, conceito que não corresponde à associação IRC a canais. Um usuário IRC pode estar em vários canais simultaneamente, enquanto `Client` guarda somente um `_active_channel`. É necessário:

- permitir associação simultânea a vários canais;
- encaminhar `PRIVMSG` de canal a todos os demais membros (o enunciado diz “every other client”);
- transmitir eventos `JOIN`, `PART`, `KICK`, `QUIT`, `NICK`, `TOPIC` e `MODE` aos destinatários apropriados;
- evitar ponteiros/associações obsoletos após desconexão.

## Erros funcionais adicionais

- `accept()` aceita no máximo um cliente por evento e interrompe a iteração; deve ser projetado de modo consistente com socket não bloqueante e eventos subsequentes.
- Não há limite de tamanho para o buffer de entrada de um cliente que nunca envia newline, permitindo crescimento indefinido.
- Não há validação de tamanho/sintaxe de nickname, username, canal ou linha IRC.
- Não há diferenciação consistente entre maiúsculas/minúsculas dos comandos; comandos IRC devem ser reconhecidos adequadamente.
- Não há numerics de erro, como senha incorreta, nickname em uso, parâmetros insuficientes, canal inexistente, usuário inexistente, não membro ou falta de privilégio.
- `create_channel()` adiciona membro e operador, mas não o coloca em `_active_members` nem atualiza `_active_channel`; portanto até o protocolo próprio exige um segundo comando para realmente “entrar”.
- `Channel::enter()` adiciona um cliente a `_active_members` mesmo quando ele não é membro/convidado, pois a atribuição ocorre fora do `if`.
- `leave()` remove somente de `_active_members`, mantendo associação permanente em `_members`; isso não corresponde a `PART`.
- `kick()` remove convite junto com membro sem uma especificação clara e não valida se o alvo está no canal antes de anunciar sucesso.
- Convite, expulsão e mudança de operador dependem do “canal ativo” do autor, em vez do canal informado no comando.
- Respostas contêm texto informal, emojis e apenas `\n`; mensagens IRC precisam de formato protocolar e terminação `\r\n`.
- `Client::read_message()` adiciona um array sem garantir terminador NUL e seria comportamento indefinido se a função fosse usada. Atualmente ela está duplicada e não é usada.
- Há código morto, comentários de trabalho pendente, métodos duplicados/conceitos redundantes e `classes/server.cpp` vazio.
- O cabeçalho global inclui praticamente todas as classes, criando dependências circulares difíceis de manter.

## README não conforme

O Git root é `IA_IRC`, mas o único README está em `IA_IRC/AI_IRC/README.md`; portanto não há `README.md` na raiz do repositório conforme exigido.

Além disso, o README atual contém apenas cinco referências a imagens inexistentes e não atende nenhum conteúdo obrigatório. O README da raiz deve estar em inglês e conter:

- primeira linha em itálico, exatamente no formato exigido: `This project has been created as part of the 42 curriculum by <logins>.`;
- seção `Description`;
- seção `Instructions` com compilação e execução;
- seção `Resources` com referências clássicas sobre IRC/TCP/poll;
- descrição de como IA foi usada, em quais tarefas e partes;
- recomendavelmente cliente IRC de referência, lista de recursos implementados e exemplos de uso/teste.

As imagens referenciadas (`img/...`) também não existem no diretório atual.

## Organização e entrega

- O binário e os arquivos `.o` aparecem como arquivos não rastreados no repositório. Eles não devem ser entregues; adicionar regras adequadas ao `.gitignore` é recomendável.
- O PDF do enunciado também aparece não rastreado; normalmente não é necessário entregá-lo.
- Confirmar que todos os fontes necessários estão efetivamente adicionados ao Git, pois somente conteúdo do repositório será avaliado.
- O Makefile deve incluir todos os arquivos finais e gerar exatamente `ircserv`.
- Seria melhor colocar o projeto diretamente na raiz Git ou garantir que o avaliador execute no diretório correto; o README, em qualquer caso, deve ficar na raiz Git.

## Ordem recomendada de implementação

1. Corrigir Makefile, argumentos, validação de porto, criação do socket e encerramento limpo.
2. Redesenhar `Server`/`Client` para guardar senha global, estado de registro, nickname, username, buffer de entrada e fila de saída.
3. Implementar parser de linhas IRC e extração correta de zero, uma ou várias linhas por `recv`.
4. ~~Implementar `poll` com `POLLIN` e `POLLOUT`, filas e envios parciais.~~ **Concluído.**
5. ~~Implementar registro `PASS` + `NICK` + `USER` e numerics essenciais.~~ **Concluído.**
6. ~~Implementar `PING`/`PONG`, `QUIT`, troca de `NICK` e erros comuns.~~ **Concluído.**
7. ~~Redesenhar canais para múltiplas associações por cliente.~~ **Concluído.**
8. ~~Implementar `JOIN`, `PART`, `PRIVMSG` para usuário/canal e broadcasts IRC.~~ **Concluído.**
9. ~~Implementar operadores, `KICK`, `INVITE`, `TOPIC` e todos os modos `i`, `t`, `k`, `o`, `l`.~~ **Concluído.**
10. Testar com o cliente IRC de referência e então completar o README em inglês.

## Testes mínimos antes de considerar concluído

- Compilação limpa com as flags obrigatórias e sem relink desnecessário.
- Porto inválido, ocupado, fora da faixa e argumentos ausentes.
- Senha correta e incorreta; ordens diferentes de `PASS`/`NICK`/`USER`.
- Nickname duplicado e troca de nickname.
- Vários clientes simultâneos sem travamento.
- Comando dividido em vários pacotes, como o exemplo `com`, `man`, `d\n` do enunciado.
- Vários comandos em um pacote e uma linha completa mais uma parcial.
- Linhas `\r\n`, somente `\n`, vazias, muito longas e comandos desconhecidos.
- Mensagem privada usuário→usuário e canal→todos os outros membros.
- Um cliente em vários canais.
- Todos os casos positivos e negativos de `KICK`, `INVITE`, `TOPIC` e `MODE` (`i`, `t`, `k`, `o`, `l`).
- Desconexão abrupta, `QUIT`, `POLLHUP`/`POLLERR` e limpeza de canais.
- Cliente lento e envio parcial sem bloquear os demais.
- Execução prolongada sob Valgrind/sanitizers durante desenvolvimento (sem transformar flags não exigidas em dependência da entrega).
- Conexão e uso completo pelo cliente IRC escolhido como referência, sem erros.

## Bonus

Transferência de arquivos e bot são opcionais. Não devem ser priorizados até toda a parte obrigatória funcionar perfeitamente, pois o bônus só é avaliado nessa condição.
