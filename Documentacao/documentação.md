# Documentação da Máquina de Cartão RFID — Banco Imobiliário

Documentação técnica do firmware Arduino da maquininha de cartões RFID.

## Resumo

A maquininha permite transações eletrônicas entre jogadores via cartões RFID. Suporta até 6 jogadores, persistência na EEPROM com validação por checksum, confirmação de operações, desfazer última ação e cadastro dinâmico de cartões.

## Arquivos do firmware

| Arquivo | Descrição |
|---|---|
| `banco.ino` | Setup, loop e menus |
| `config.h` | Constantes, enums e estruturas de dados |
| `storage.h` | Leitura/escrita EEPROM, cartões e histórico |
| `utils.h` | Timer não bloqueante e funções de string |

## Componentes

1. **MFRC522** — Leitor RFID
2. **Teclado matricial 4×4**
3. **Display LCD I2C 16×2**
4. **EEPROM** — Persistência de dados

## Configuração de pinos

### RFID

- SS_PIN (Chip Select): 10
- RST_PIN (Reset): 9

### Teclado matricial

- Linhas: 2, 3, 4, 5
- Colunas: A0, A1, A2, A3

### Display LCD I2C

- Endereço: 0x27
- Tamanho: 16×2

## Variáveis importantes (`config.h`)

| Variável | Padrão | Descrição |
|---|---|---|
| `MODO_SIMULADOR` | false | Ativa modo de teste no Wokwi |
| `DINHEIRO_INICIAL_PADRAO` | 400 | Saldo inicial de cada jogador |
| `SALARIO_PASSAGEM_PADRAO` | 200 | Valor ao passar pelo início |
| `QTD_MAXIMA_JOGADORES` | 6 | Limite de jogadores |
| `TEMPO_TELA_MS` | 1000 | Duração de mensagens no LCD |
| `TIMEOUT_INATIVIDADE_MS` | 300000 | Retorno ao menu após 5 min |

## Estruturas de dados

### `CartaoInfo`

Armazena UID hexadecimal e nome fantasia do cartão RFID.

### `ContaJogador`

- `id` — índice no array de cartões
- `saldo` — saldo em reais (`int32_t`, sem decimais)

### `OpRealizada`

Dados para desfazer a última operação (tipo, jogadores, valor).

### `EEPROMHeader`

Cabeçalho com magic number, versão, quantidade de jogadores, valores configurados e checksum.

### `HistoricoEntry`

Registro de transação no ring buffer (últimas 10 operações).

## Layout da EEPROM

| Offset | Conteúdo |
|---|---|
| 0 | Header (magic, versão, configurações, checksum) |
| Header + | Array de jogadores (até 6) |
| Histórico | Ring buffer de 10 transações |
| Cartões | Quantidade + array de cartões cadastrados |

## Estados do menu

| Estado | Função |
|---|---|
| `MENU_INICIO` | Novo jogo / continuar / configurações |
| `MENU_QTD_JOGADORES` | Seleção de 2 a 6 jogadores |
| `MENU_ESPERANDO_CARTAO` | Cadastro de cartões RFID |
| `MENU_CALCULADORA` | Entrada de valor e seleção de operação |
| `MENU_CONFIRMACAO` | Confirmação antes de executar |
| `MENU_ADICIONAR` | Aguarda cartão para crédito |
| `MENU_RETIRAR` | Aguarda cartão para débito |
| `MENU_TRANSFERIR_ORIGEM` | Cartão do pagador |
| `MENU_TRANSFERIR_DESTINO` | Cartão do recebedor |
| `MENU_SALARIO` | Crédito de salário (passagem pelo início) |
| `MENU_LISTA_JOGADORES` | Exibição sequencial de saldos |

## Fluxo de operações

1. Jogador digita valor na calculadora
2. Pressiona +, − ou C (transferir)
3. Tela de confirmação exibe valor e operação
4. Confirma com **#** ou cancela com **D**
5. Aproxima cartão(ões) RFID
6. Saldo atualizado e gravado na EEPROM

### Transferência

A transferência usa dois estados separados (sem recursão):

1. Lê cartão de origem e valida saldo
2. Lê cartão de destino (deve ser diferente)
3. Débito e crédito ocorrem apenas após leitura do destino

### Desfazer

Com a tela de valor vazia, pressione **D** para reverter a última operação (+, −, T ou S).

## Teclado — mapa completo

```
[ 1 ] [ 2 ] [ 3 ] [ + ]  → Adicionar
[ 4 ] [ 5 ] [ 6 ] [ - ]  → Retirar
[ 7 ] [ 8 ] [ 9 ] [ C ]  → Transferir
[ * ] [ 0 ] [ # ] [ D ]  → Lista / Salário / Confirmar / Apagar
```

## Cartões pré-cadastrados

| UID | Nome |
|---|---|
| a7848236 | branco |
| b9d0d693 | azul |
| 040a71b2dc4c81 | visa |
| 040271b2dc4c81 | master |

Cartões desconhecidos são cadastrados automaticamente como "Jog.N".

## Uso

1. Ligue a maquininha e aguarde a tela de carregamento.
2. Escolha novo jogo ou continuar partida salva.
3. Configure dinheiro inicial (#) e salário (*) se necessário.
4. Selecione quantidade de jogadores e cadastre os cartões.
5. Realize transações durante o jogo.
6. Use **D** para desfazer erros.
7. Use **D** no menu inicial para apagar dados salvos.

## Considerações

- Valores monetários são inteiros (sem centavos) para evitar erros de ponto flutuante.
- Strings dinâmicas foram substituídas por buffers `char[]` para reduzir fragmentação de memória.
- Timers com `millis()` substituem `delay()` bloqueante nas operações principais.
- Altere `MODO_SIMULADOR` para `true` apenas ao testar no Wokwi.
