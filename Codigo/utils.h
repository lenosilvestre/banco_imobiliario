#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

class Timer {
  unsigned long inicioMs;
  unsigned long duracaoMs;
  bool ativo;

public:
  Timer() : inicioMs(0), duracaoMs(0), ativo(false) {}

  void iniciar(unsigned long ms) {
    inicioMs = millis();
    duracaoMs = ms;
    ativo = true;
  }

  bool expirou() const {
    return ativo && (millis() - inicioMs >= duracaoMs);
  }

  void parar() { ativo = false; }
  bool estaAtivo() const { return ativo; }
};

inline void limparBuffer(char* buf, int tamanho) {
  for (int i = 0; i < tamanho; i++) {
    buf[i] = '\0';
  }
}

inline int tamanhoString(const char* buf) {
  int len = 0;
  while (buf[len] != '\0') len++;
  return len;
}

inline void adicionarDigito(char* buf, int tamanhoMax, char digito) {
  int len = tamanhoString(buf);
  if (len < tamanhoMax - 1) {
    buf[len] = digito;
    buf[len + 1] = '\0';
  }
}

inline void removerUltimoDigito(char* buf) {
  int len = tamanhoString(buf);
  if (len > 0) {
    buf[len - 1] = '\0';
  }
}

inline long stringParaLong(const char* buf) {
  long valor = 0;
  for (int i = 0; buf[i] != '\0'; i++) {
    if (buf[i] >= '0' && buf[i] <= '9') {
      valor = valor * 10 + (buf[i] - '0');
    }
  }
  return valor;
}

inline void copiarString(char* dest, int tamanhoDest, const char* origem) {
  int i = 0;
  while (origem[i] != '\0' && i < tamanhoDest - 1) {
    dest[i] = origem[i];
    i++;
  }
  dest[i] = '\0';
}

inline bool stringsIguais(const char* a, const char* b) {
  int i = 0;
  while (a[i] != '\0' && b[i] != '\0') {
    if (a[i] != b[i]) return false;
    i++;
  }
  return a[i] == b[i];
}

#endif
