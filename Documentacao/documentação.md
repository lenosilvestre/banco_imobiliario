# Documentação da Máquina de Cartão RFID para o Jogo de Tabuleiro Banco Imobiliário

Esta documentação descreve o código Arduino utilizado para criar uma máquina de cartão RFID para uma versão do jogo de tabuleiro Banco Imobiliário. O código utiliza os componentes RFID, teclado matricial, display LCD I2C, e EEPROM para realizar operações de adição, retirada e transferência de dinheiro entre jogadores.

## Resumo

A máquina de cartão RFID para o jogo de tabuleiro Banco Imobiliário permite aos jogadores realizar transações de dinheiro de forma eletrônica utilizando cartões RFID personalizados. Os jogadores podem adicionar, retirar e transferir dinheiro entre si, tudo isso com a facilidade de aproximar um cartão RFID.

## Componentes Utilizados

1. RFID Module (MFRC522)
2. Teclado Matricial
3. Display LCD I2C
4. EEPROM (para armazenar dados dos jogadores)

## Configuração

### Pinos RFID

- SS_PIN (Chip Select): 10
- RST_PIN (Reset): 9

### Pinos Teclado Matricial

- Linhas: 4, 3, 2, 5
- Colunas: A0, A1, A2, A3

### Display LCD I2C

- Endereço I2C: 0x27
- Tamanho: 16x2

## Variáveis Importantes

- `dinheiroInicial`: Define o dinheiro inicial de cada jogador.
- `tempoDeTela`: Duração das mensagens exibidas no LCD em milissegundos.
- `qtdMaximaDeJogadores`: Define o número máximo de jogadores (cartões RFID) que podem participar.

## Estruturas de Dados

1. `nomeCartoes`: Estrutura para nomear os cartões RFID.
2. `contaJogadores`: Estrutura para armazenar informações dos jogadores.
3. `opRealizada`: Estrutura para desfazer a última operação.

## Funções Principais

### `setup()`

- Inicializa a comunicação SPI para o RFID.
- Inicializa o display LCD.
- Cria caracteres especiais para exibição no LCD.
- Chama a função `telaDeCarregamento()`.

### `loop()`

- Implementa um menu de operações: início, seleção de jogadores, aguardando cartões, calculadora, adicionar dinheiro, retirar dinheiro e transferir dinheiro.

### `menuDeInicio()`

- Exibe opções para iniciar um novo jogo ou continuar um jogo salvo.
- Permite configurar o dinheiro inicial.

### `qtdJogadores()`

- Permite ao usuário selecionar a quantidade de jogadores para o jogo.

### `esperandoCartao()`

- Aguarda os jogadores aproximarem os cartões RFID e adiciona-os à lista de jogadores.
- Exibe os nomes fantasia dos jogadores e seus saldos.

### `calculadora()`

- Permite aos jogadores digitar valores e selecionar operações (adicionar, retirar, transferir).
- Exibe o saldo atualizado no LCD.

### `operacaoAdicionar()`

- Adiciona dinheiro à conta de um jogador.

### `operacaoRetirar()`

- Retira dinheiro da conta de um jogador.

### `operacaoTransferir()`

- Permite transferir dinheiro de um jogador para outro.

### `operacaoDesfazer()`

- Desfaz a última operação realizada.

### `mostraNovoSaldo()`

- Exibe o novo saldo de um jogador no LCD e salva na EEPROM.

### `exibeLcd()`

- Exibe uma mensagem no LCD com parâmetros fornecidos.

### `telaDeCarregamento()`

- Exibe uma tela de carregamento no LCD.

## Uso

1. Inicie a máquina de cartão RFID e siga as instruções no LCD.
2. Selecione o número de jogadores.
3. Aproxime os cartões RFID dos jogadores para adicioná-los ao jogo.
4. Utilize o teclado matricial para realizar transações de dinheiro.
5. Desfazer a última operação, se necessário.
6. Continue jogando ou encerre o jogo.

## Considerações Finais

Este código Arduino permite a criação de uma máquina de cartão RFID para a versão eletrônica do jogo de tabuleiro Banco Imobiliário. Ele oferece funcionalidades para adicionar, retirar e transferir dinheiro entre jogadores de forma fácil e rápida. Certifique-se de conectar corretamente os componentes e de adaptar o código conforme necessário para seu projeto específico.
