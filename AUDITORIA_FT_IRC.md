# Auditoria final do projeto `ft_irc`

## Resultado

**Estado atual: a parte obrigatória está implementada e o projeto está em condição de candidato à entrega.**

A implementação foi comparada com `en.subject.pdf` versão 11.0, compilada em C++98 e testada com clientes TCP, HexChat, AddressSanitizer e UndefinedBehaviorSanitizer.

Antes da entrega, o autor ainda deve colocar o projeto no repositório Git correto da 42 e enviar a branch de entrega.

## Requisitos gerais

| Requisito | Estado | Evidência |
|---|---|---|
| Executável chamado `ircserv` | Concluído | `NAME := ircserv` no Makefile |
| Execução com porta e senha | Concluído | `./ircserv <port> <password>` |
| C++98 | Concluído | `-std=c++98 -pedantic` |
| Warnings como erros | Concluído | `-Wall -Wextra -Werror` |
| Regras `all`, `clean`, `fclean`, `re` | Concluído | Makefile |
| Sem relink desnecessário | Concluído | segundo `make` não executa comandos |
| Dependências de cabeçalhos | Concluído | arquivos `.d` com `-MMD -MP` |
| Sem bibliotecas externas | Concluído | somente biblioteca padrão e API de sockets |

## Inicialização e rede

| Requisito | Estado |
|---|---|
| Validação da quantidade de argumentos | Concluído |
| Porta numérica entre 1 e 65535 | Concluído |
| Senha não vazia | Concluído |
| TCP/IPv4 | Concluído |
| `SO_REUSEADDR` | Concluído |
| Testes de falha de `socket`, `setsockopt`, `bind`, `fcntl` e `listen` | Concluído |
| Socket de escuta não bloqueante | Concluído |
| Sockets dos clientes não bloqueantes | Concluído |
| Encerramento por `SIGINT` e `SIGTERM` | Concluído |
| Proteção contra `SIGPIPE` | Concluído |

## Uso de `poll()`

- existe um único loop central baseado em `poll()`;
- o socket de escuta é aceito somente após `POLLIN`;
- `recv()` aparece em um único ponto e é chamado após `POLLIN`;
- `send()` aparece em um único ponto e é chamado após `POLLOUT`;
- `POLLERR`, `POLLNVAL` e `POLLHUP` são tratados;
- quando `POLLIN` e `POLLHUP` chegam juntos, os dados são lidos primeiro;
- respostas pendentes podem ser enviadas antes do fechamento;
- a implementação não repete operações com base em `errno == EAGAIN`.

## Entrada TCP

- cada cliente possui buffer próprio;
- fragmentos são preservados entre chamadas de `recv()`;
- todas as linhas completas de um pacote são processadas em ordem;
- `\r\n` e `\n` são aceitos;
- o tamanho acumulado possui limite de segurança;
- o parser separa prefixo, comando, parâmetros e parâmetro final iniciado por `:`;
- comandos são reconhecidos sem diferenciar maiúsculas/minúsculas.

Teste fragmentado executado:

```text
PA + SS secret
NI + CK fragment
PING split + -token
```

Resultado: registro concluído e `PONG` correto, sem erro dos sanitizers.

## Saída TCP

- handlers apenas enfileiram respostas;
- `POLLOUT` é habilitado somente enquanto há dados;
- somente a quantidade efetivamente enviada é removida da fila;
- o sufixo de um envio parcial permanece para o próximo evento;
- a fila é limitada a 1 MiB por cliente;
- clientes lentos que excedem o limite são desconectados.

## Registro IRC

| Comando/recurso | Estado |
|---|---|
| `PASS` com senha global | Concluído |
| `NICK` inicial | Concluído |
| Nickname único | Concluído |
| Alteração de `NICK` | Concluído |
| `USER` e realname | Concluído |
| Ordem flexível de `PASS`, `NICK`, `USER` | Concluído |
| Numeric `001` | Concluído |
| Rejeição de comandos antes do registro | Concluído |
| Erros de senha, nickname e parâmetros | Concluído |

## Comandos de conexão e compatibilidade

| Comando | Estado |
|---|---|
| `PING` / `PONG` | Concluído |
| `QUIT` com motivo | Concluído |
| `CAP LS`, `CAP LIST`, `CAP REQ`, `CAP END` | Concluído |
| `NAMES` | Concluído |
| `WHO` | Concluído |
| `NOTICE` | Concluído |
| Comandos desconhecidos | Numeric `421` |

HexChat foi usado como cliente de referência e conseguiu conectar dois usuários, entrar em canais, trocar mensagens e consultar modos.

## Canais e mensagens

| Recurso | Estado |
|---|---|
| Cliente em vários canais | Concluído |
| Criação automática no primeiro `JOIN` | Concluído |
| Primeiro membro torna-se operador | Concluído |
| `JOIN` simples e múltiplo | Concluído |
| `JOIN 0` | Concluído |
| `PART` simples e múltiplo | Concluído |
| Mensagem privada usuário→usuário | Concluído |
| `PRIVMSG` para canal | Concluído |
| Remetente excluído do broadcast de `PRIVMSG` | Concluído |
| Eventos `JOIN`, `PART`, `QUIT` e `NICK` | Concluído |
| Remoção de canais vazios | Concluído |

## Operadores e modos obrigatórios

| Recurso | Estado |
|---|---|
| `KICK` | Concluído |
| `INVITE` | Concluído |
| Consulta e alteração de `TOPIC` | Concluído |
| Consulta de `MODE` | Concluído |
| `+i` / `-i` | Concluído |
| `+t` / `-t` | Concluído |
| `+k` / `-k` | Concluído |
| `+o` / `-o` | Concluído |
| `+l` / `-l` | Concluído |
| Combinações como `+it` e `+kl` | Concluído |
| Verificação de operador | Concluído |
| Chave, convite e limite aplicados no `JOIN` | Concluído |

## Desconexões

- `QUIT` notifica os membros afetados;
- respostas pendentes são esvaziadas antes do fechamento controlado;
- EOF, falha de leitura/escrita e eventos de erro removem o cliente;
- o cliente é removido de todos os canais;
- canais vazios são apagados;
- nicknames são liberados para reutilização;
- destinatários compartilhados por vários canais são deduplicados.

## Documentação

| Documento | Estado |
|---|---|
| README em inglês | Concluído |
| Primeira linha obrigatória | Preenchida com o login confirmado `dleite-b` |
| `Description` | Concluído |
| `Instructions` | Concluído |
| `Resources` | Concluído |
| Explicação do uso de IA | Concluído |
| Manual de uso | `MANUAL_DE_USO.md` |
| Fluxo do programa | `FLUXO_DO_PROGRAMA.md` |

## Organização Git

- o repositório Git local criado durante o desenvolvimento não pertence à escola e foi removido;
- `.gitignore` permanece no projeto para ignorar objetos, dependências, binários, logs e o PDF;
- ainda é necessário usar o repositório oficial da 42 para a entrega.

Ainda é necessário:

1. copiar ou adicionar o projeto ao repositório oficial da 42;
2. revisar `git status`;
3. criar o commit de entrega;
4. enviar a branch para o remote oficial.

## Testes executados nesta revisão

- `make fclean && make`;
- segundo `make` sem relink;
- argumentos ausentes;
- porta inválida;
- senha vazia;
- compilação com AddressSanitizer e UndefinedBehaviorSanitizer;
- negociação `CAP`;
- registro IRC;
- `JOIN`, `NAMES` e `WHO`;
- `NOTICE` e `PING`;
- dois clientes simultâneos;
- canal invite-only, chave, limite e proteção do tópico;
- convite, promoção de operador e mensagens;
- comandos enviados em vários fragmentos;
- vários comandos no mesmo fluxo TCP;
- fechamento imediato da entrada com respostas ainda pendentes.

Nenhum erro de AddressSanitizer ou UndefinedBehaviorSanitizer foi observado.

## Limitações e observações finais

- Valgrind não está instalado no ambiente; os sanitizers foram usados como alternativa.
- Casos de falha real por falta total de memória não foram reproduzidos.
- Transferência de arquivos e bot são bônus e não foram implementados.
- O remote e o push dependem das credenciais/repositório fornecidos pela 42.

## Checklist antes da defesa

- [x] Confirmar login(s) na primeira linha do README (`dleite-b`).
- [ ] Configurar o remote oficial.
- [ ] Configurar o remote e fazer push.
- [ ] Clonar o repositório em outro diretório e recompilar.
- [ ] Repetir o teste completo no HexChat.
- [ ] Saber explicar `poll`, buffers, parser, filas e modos.
- [ ] Treinar uma pequena modificação ao vivo.
