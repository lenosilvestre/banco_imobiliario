# Maquininha de Cartões para o jogo de tabuleiro Banco Imobiliário

Este é um projeto de maquininha de cartões utilizando Arduino para ser utilizado no jogo de tabuleiro Banco Imobiliário. O objetivo do projeto é oferecer uma forma de gerenciar as transações financeiras dos jogadores, tornando o jogo mais prático e divertido.

## Funcionalidades

A maquininha possui as seguintes funcionalidades:

- Leitura de cartões RFID para identificar os jogadores
- Cálculo de transações financeiras (adicionar, retirar e transferir dinheiro)
- Exibição do saldo dos jogadores em um display LCD
- Interface com teclado matricial para inserção de valores

## Bibliotecas utilizadas

O projeto utiliza as seguintes bibliotecas do Arduino:

- Keypad.h
- LiquidCrystal.h
- IRremote.h
- Thread.h
- ThreadController.h
- EEPROM.h

## Como utilizar

Para utilizar a maquininha de cartões no jogo de tabuleiro Banco Imobiliário, siga os passos abaixo:

1. Conecte o Arduino ao computador e carregue o código da maquininha.
2. Conecte o leitor RFID ao Arduino e certifique-se de que ele está funcionando corretamente.
3. Conecte o display LCD ao Arduino e certifique-se de que ele está funcionando corretamente.
4. Conecte o teclado matricial ao Arduino e certifique-se de que ele está funcionando corretamente.
5. Inicie o jogo e solicite que cada jogador passe seu cartão RFID na maquininha para que seu saldo seja cadastrado.
6. Durante o jogo, utilize a maquininha para realizar as transações financeiras necessárias (adicionar, retirar e transferir dinheiro).
7. Ao final do jogo, a lista de jogadores e seus respectivos saldos será armazenada na memória EEPROM do Arduino para ser utilizada em jogos futuros.

## Limitações

- A maquininha suporta até 6 jogadores.
- A memória EEPROM do Arduino tem uma capacidade limitada e pode se esgotar após muitos jogos.

## Contribuições

Contribuições para o projeto são sempre bem-vindas. Caso encontre algum problema ou tenha alguma sugestão de melhoria, fique à vontade para criar uma issue ou submeter um pull request.

## Licença

Este projeto é distribuído sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.

(Readme criado pelo Chat GPT3)
