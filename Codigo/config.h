#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Altere para true apenas ao testar no simulador Wokwi
const bool MODO_SIMULADOR = false;

const long DINHEIRO_INICIAL_PADRAO = 400;
const long SALARIO_PASSAGEM_PADRAO = 200;
const unsigned long TEMPO_TELA_MS = 1000;
const unsigned long TIMEOUT_INATIVIDADE_MS = 300000; // 5 minutos

const int QTD_MAXIMA_JOGADORES = 6;
const int QTD_MAXIMA_CARTOES = 10;
const int MAX_DIGITOS_VALOR = 8;
const int HISTORICO_TAMANHO = 10;

const int EEPROM_MAGIC = 0xB1C0;
const int EEPROM_VERSAO = 2;
const int EEPROM_ENDERECO_BASE = 0;

// Pinos RFID
const byte SS_PIN = 10;
const byte RST_PIN = 9;

// Pinos teclado matricial
const byte LINHAS_TECLADO = 4;
const byte COLUNAS_TECLADO = 4;

// Display I2C
const int ENDERECO_I2C = 0x27;

// Estados do menu
enum MenuEstado {
  MENU_INICIO = 0,
  MENU_QTD_JOGADORES,
  MENU_ESPERANDO_CARTAO,
  MENU_CALCULADORA,
  MENU_ADICIONAR,
  MENU_RETIRAR,
  MENU_TRANSFERIR_ORIGEM,
  MENU_TRANSFERIR_DESTINO,
  MENU_CONFIRMACAO,
  MENU_CONFIG_DINHEIRO,
  MENU_CONFIG_SALARIO,
  MENU_RESET_CONFIRM,
  MENU_SALARIO,
  MENU_MSG_TEMPORARIA,
  MENU_LISTA_JOGADORES
};

enum OperacaoPendente {
  OP_NENHUMA = 0,
  OP_ADICIONAR,
  OP_RETIRAR,
  OP_TRANSFERIR,
  OP_SALARIO
};

struct CartaoInfo {
  char codCartao[18];
  char nomeFantasia[10];
};

struct ContaJogador {
  int8_t id;
  int32_t saldo;
};

struct OpRealizada {
  char operador; // '+', '-', 'T', 'S'
  int8_t posJog1;
  int8_t posJog2;
  int32_t valor;
};

struct HistoricoEntry {
  char tipo;
  int8_t jog1;
  int8_t jog2;
  int32_t valor;
};

struct EEPROMHeader {
  uint16_t magic;
  uint8_t versao;
  uint8_t qtdJogadores;
  int32_t dinheiroInicial;
  int32_t salarioPassagem;
  uint8_t qtdCartoesCadastrados;
  uint16_t checksum;
};

#endif
