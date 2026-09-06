# Maquininha de Cartões para o jogo de tabuleiro Banco Imobiliário

Projeto de maquininha de cartões com Arduino para gerenciar transações financeiras no jogo de tabuleiro Banco Imobiliário. Cada jogador possui um cartão RFID vinculado à sua conta virtual.

## Funcionalidades

- Leitura de cartões RFID para identificar jogadores (até 6)
- Cadastro automático de cartões RFID desconhecidos
- Operações financeiras: adicionar, retirar e transferir dinheiro
- Atalho de salário ao passar pelo ponto de partida
- Confirmação antes de executar transações
- Desfazer última operação
- Exibição de saldos no display LCD I2C
- Interface com teclado matricial 4×4
- Persistência na EEPROM com checksum e validação
- Histórico das últimas 10 transações
- Continuar jogo salvo ou iniciar novo
- Reset de dados salvos

## Hardware

| Componente | Detalhes |
|---|---|
| Arduino Uno/Nano | Microcontrolador |
| MFRC522 | Leitor RFID (SS=10, RST=9) |
| LCD I2C 16×2 | Endereço 0x27 |
| Teclado 4×4 | Linhas: 2,3,4,5 — Colunas: A0–A3 |

## Bibliotecas

- Keypad.h
- LiquidCrystal_I2C.h
- EEPROM.h
- MFRC522.h
- SPI.h
- Wire.h

## Estrutura do código

```
Codigo/
├── banco.ino    # Firmware principal
├── config.h     # Constantes e estruturas
├── storage.h    # EEPROM, cartões e histórico
└── utils.h      # Utilitários de string e timer
```

## Teclado

| Tecla | Função |
|---|---|
| 0–9 | Digitar valor |
| + | Adicionar dinheiro |
| − | Retirar dinheiro |
| C | Transferir dinheiro |
| * | Listar saldos |
| # | Salário (tela vazia) / Confirmar |
| D | Apagar dígito / Desfazer operação |

### Menu inicial

| Tecla | Função |
|---|---|
| 1 | Novo jogo |
| 2 | Continuar jogo salvo |
| # | Configurar dinheiro inicial |
| * | Configurar valor do salário |
| D | Apagar dados salvos |

## Valores padrão

- Dinheiro inicial: **R$ 400**
- Salário ao passar pelo início: **R$ 200**
- Máximo de jogadores: **6**

## Simulador Wokwi

[WOKWI Simulador](https://wokwi.com/projects/372779675952274433)

Para testar no simulador, altere em `config.h`:

```cpp
const bool MODO_SIMULADOR = true;
```

## Como utilizar

1. Carregue o código no Arduino e conecte os componentes.
2. No menu inicial, escolha **1** para novo jogo ou **2** para continuar.
3. Selecione a quantidade de jogadores (2 a 6).
4. Cada jogador aproxima seu cartão RFID para cadastro.
5. Durante o jogo, digite o valor e escolha a operação (+, −, C).
6. Confirme com **#** e aproxime o cartão do jogador.
7. Para salário ao passar pelo início, pressione **#** com a tela vazia.

## Limitações

- Máximo de 6 jogadores e 10 cartões cadastrados
- Memória EEPROM limitada (dados persistem entre sessões)
- Desfazer apenas a última operação

## Licença

Este projeto é distribuído sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.
