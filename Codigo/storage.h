#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"
#include "utils.h"
#include <EEPROM.h>

const int EEPROM_HEADER_SIZE = sizeof(EEPROMHeader);
const int EEPROM_JOGADORES_OFFSET = EEPROM_HEADER_SIZE;
const int EEPROM_HISTORICO_OFFSET = EEPROM_JOGADORES_OFFSET + (QTD_MAXIMA_JOGADORES * sizeof(ContaJogador));
const int EEPROM_CARTOES_OFFSET = EEPROM_HISTORICO_OFFSET + (HISTORICO_TAMANHO * sizeof(HistoricoEntry));

extern ContaJogador players[QTD_MAXIMA_JOGADORES];
extern CartaoInfo cartoes[QTD_MAXIMA_CARTOES];
extern int qtdCartoesCadastrados;
extern int qtdDeJogadores;
extern long dinheiroInicial;
extern long salarioPassagem;
extern HistoricoEntry historico[HISTORICO_TAMANHO];
extern int historicoIndice;

inline uint16_t calcularChecksum(const EEPROMHeader& header, const ContaJogador* jogadores) {
  uint16_t sum = header.magic + header.versao + header.qtdJogadores;
  sum += (uint16_t)(header.dinheiroInicial & 0xFFFF);
  sum += (uint16_t)((header.dinheiroInicial >> 16) & 0xFFFF);
  sum += (uint16_t)(header.salarioPassagem & 0xFFFF);
  sum += (uint16_t)((header.salarioPassagem >> 16) & 0xFFFF);
  sum += header.qtdCartoesCadastrados;

  for (int i = 0; i < header.qtdJogadores; i++) {
    sum += jogadores[i].id;
    sum += (uint16_t)(jogadores[i].saldo & 0xFFFF);
    sum += (uint16_t)((jogadores[i].saldo >> 16) & 0xFFFF);
  }
  return sum;
}

inline void inicializarCartoesPadrao() {
  qtdCartoesCadastrados = 5;
  copiarString(cartoes[0].codCartao, 18, "NULNUL");
  copiarString(cartoes[0].nomeFantasia, 10, "NULNUL");
  copiarString(cartoes[1].codCartao, 18, "a7848236");
  copiarString(cartoes[1].nomeFantasia, 10, "branco");
  copiarString(cartoes[2].codCartao, 18, "b9d0d693");
  copiarString(cartoes[2].nomeFantasia, 10, "azul");
  copiarString(cartoes[3].codCartao, 18, "040a71b2dc4c81");
  copiarString(cartoes[3].nomeFantasia, 10, "visa");
  copiarString(cartoes[4].codCartao, 18, "040271b2dc4c81");
  copiarString(cartoes[4].nomeFantasia, 10, "master");
}

inline void limparJogadores() {
  for (int i = 0; i < QTD_MAXIMA_JOGADORES; i++) {
    players[i].id = -1;
    players[i].saldo = 0;
  }
}

inline void registrarHistorico(char tipo, int8_t jog1, int8_t jog2, int32_t valor) {
  historico[historicoIndice].tipo = tipo;
  historico[historicoIndice].jog1 = jog1;
  historico[historicoIndice].jog2 = jog2;
  historico[historicoIndice].valor = valor;
  historicoIndice = (historicoIndice + 1) % HISTORICO_TAMANHO;

  int offset = EEPROM_HISTORICO_OFFSET + historicoIndice * sizeof(HistoricoEntry);
  EEPROM.put(offset, historico[historicoIndice]);
}

inline void salvarCartoesEEPROM() {
  EEPROM.put(EEPROM_CARTOES_OFFSET, qtdCartoesCadastrados);
  for (int i = 0; i < qtdCartoesCadastrados; i++) {
    int offset = EEPROM_CARTOES_OFFSET + (int)sizeof(int) + i * (int)sizeof(CartaoInfo);
    EEPROM.put(offset, cartoes[i]);
  }
}

inline void carregarCartoesEEPROM() {
  EEPROM.get(EEPROM_CARTOES_OFFSET, qtdCartoesCadastrados);
  if (qtdCartoesCadastrados <= 0 || qtdCartoesCadastrados > QTD_MAXIMA_CARTOES) {
    inicializarCartoesPadrao();
    return;
  }
  for (int i = 0; i < qtdCartoesCadastrados; i++) {
    int offset = EEPROM_CARTOES_OFFSET + (int)sizeof(int) + i * (int)sizeof(CartaoInfo);
    EEPROM.get(offset, cartoes[i]);
  }
}

inline void salvarNaEEPROM() {
  EEPROMHeader header;
  header.magic = EEPROM_MAGIC;
  header.versao = EEPROM_VERSAO;
  header.qtdJogadores = qtdDeJogadores;
  header.dinheiroInicial = dinheiroInicial;
  header.salarioPassagem = salarioPassagem;
  header.qtdCartoesCadastrados = qtdCartoesCadastrados;
  header.checksum = calcularChecksum(header, players);

  EEPROM.put(EEPROM_ENDERECO_BASE, header);

  for (int i = 0; i < qtdDeJogadores; i++) {
    int offset = EEPROM_JOGADORES_OFFSET + i * sizeof(ContaJogador);
    EEPROM.put(offset, players[i]);
  }

  salvarCartoesEEPROM();
}

inline bool carregarDaEEPROM() {
  EEPROMHeader header;
  EEPROM.get(EEPROM_ENDERECO_BASE, header);

  if (header.magic != EEPROM_MAGIC || header.versao != EEPROM_VERSAO) {
    return false;
  }

  ContaJogador temp[QTD_MAXIMA_JOGADORES];
  for (int i = 0; i < header.qtdJogadores; i++) {
    int offset = EEPROM_JOGADORES_OFFSET + i * sizeof(ContaJogador);
    EEPROM.get(offset, temp[i]);
  }

  if (header.checksum != calcularChecksum(header, temp)) {
    return false;
  }

  qtdDeJogadores = header.qtdJogadores;
  dinheiroInicial = header.dinheiroInicial;
  salarioPassagem = header.salarioPassagem;

  for (int i = 0; i < qtdDeJogadores; i++) {
    players[i] = temp[i];
  }

  carregarCartoesEEPROM();
  return true;
}

inline void apagarEEPROM() {
  for (int i = 0; i < 256; i++) {
    EEPROM.write(i, 0);
  }
  qtdDeJogadores = 0;
  limparJogadores();
  historicoIndice = 0;
  inicializarCartoesPadrao();
}

inline int cadastrarCartaoDinamico(const char* uid) {
  for (int i = 1; i < qtdCartoesCadastrados; i++) {
    if (stringsIguais(cartoes[i].codCartao, uid)) {
      return i;
    }
  }

  if (qtdCartoesCadastrados >= QTD_MAXIMA_CARTOES) {
    return -1;
  }

  int idx = qtdCartoesCadastrados;
  copiarString(cartoes[idx].codCartao, 18, uid);
  char nome[10];
  nome[0] = 'J';
  nome[1] = 'o';
  nome[2] = 'g';
  nome[3] = '.';
  nome[4] = '0' + idx;
  nome[5] = '\0';
  copiarString(cartoes[idx].nomeFantasia, 10, nome);
  qtdCartoesCadastrados++;
  salvarCartoesEEPROM();
  return idx;
}

#endif
