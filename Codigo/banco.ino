#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#include "config.h"
#include "utils.h"
#include "storage.h"

// Hardware
MFRC522 mfrc522(SS_PIN, RST_PIN);
byte linhaPinos[] = { 2, 3, 4, 5 };
byte colunaPinos[] = { A0, A1, A2, A3 };
char teclas[] = { '1', '2', '3', '+', '4', '5', '6', '-', '7', '8', '9', 'C', '*', '0', '#', 'D' };
Keypad keypad = Keypad(makeKeymap(teclas), linhaPinos, colunaPinos, LINHAS_TECLADO, COLUNAS_TECLADO);
LiquidCrystal_I2C lcd(ENDERECO_I2C, 16, 2);

// Estado global
ContaJogador players[QTD_MAXIMA_JOGADORES];
CartaoInfo cartoes[QTD_MAXIMA_CARTOES];
HistoricoEntry historico[HISTORICO_TAMANHO];

int qtdCartoesCadastrados = 0;
int qtdDeJogadores = 0;
long dinheiroInicial = DINHEIRO_INICIAL_PADRAO;
long salarioPassagem = SALARIO_PASSAGEM_PADRAO;
int historicoIndice = 0;

MenuEstado menuOp = MENU_INICIO;
OperacaoPendente operacaoPendente = OP_NENHUMA;
OpRealizada dadosDesfazer = { '\0', -1, -1, 0 };

char valorTela[MAX_DIGITOS_VALOR + 1] = "";
long valorTransacao = 0;

MenuEstado menuRetornoMsg = MENU_INICIO;

int indiceCadastroCartao = 0;
bool aguardandoExibicaoCadastro = false;
int indiceListaJogadores = 0;
int8_t transferenciaOrigem = -1;
int auxSimulador = 1;

Timer timerMensagem;
Timer timerInatividade;
char mensagemTemporaria[17] = "";
char textoEmTela[17] = "";

byte charAAcentuado[8] = {
  B00001, B00010, B01111, B00001, B01111, B10001, B01111, B00000
};
byte charBloco[8] = {
  B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111
};

// Prototipos
void menuDeInicio();
void menuQtdJogadores();
void menuEsperandoCartao();
void menuCalculadora();
void menuAdicionar();
void menuRetirar();
void menuTransferirOrigem();
void menuTransferirDestino();
void menuConfirmacao();
void menuConfigDinheiro();
void menuConfigSalario();
void menuResetConfirm();
void menuSalario();
void menuMsgTemporaria();
void menuListaJogadores();

void resetarEstadoNovoJogo();
void exibirMensagemTemporaria(const char* msg, unsigned long ms, MenuEstado retorno);
void exibeLcd(int col, int lin, const char* texto);
void mostraNovoSaldo(int posicao);
void operacaoDesfazer();
void telaDeCarregamento();
int aproximaCartao();
int procuraJogador(int codCartao);
int procuraCartaoPorUid(const char* uid);
void mostrarErroCartao();
void limparOperacaoPendente();
void registrarOperacaoDesfazer(char op, int8_t j1, int8_t j2, long valor);
void atualizarAtividade();

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.createChar(1, charAAcentuado);
  lcd.createChar(0, charBloco);

  inicializarCartoesPadrao();
  limparJogadores();

  telaDeCarregamento();
  exibeLcd(0, 0, "Banco Imob.");
  timerInatividade.iniciar(TIMEOUT_INATIVIDADE_MS);
}

void loop() {
  if (timerInatividade.expirou()) {
    menuOp = MENU_INICIO;
    timerInatividade.iniciar(TIMEOUT_INATIVIDADE_MS);
    lcd.clear();
  }

  if (menuOp == MENU_MSG_TEMPORARIA) {
    menuMsgTemporaria();
    return;
  }

  switch (menuOp) {
    case MENU_INICIO: menuDeInicio(); break;
    case MENU_QTD_JOGADORES: menuQtdJogadores(); break;
    case MENU_ESPERANDO_CARTAO: menuEsperandoCartao(); break;
    case MENU_CALCULADORA: menuCalculadora(); break;
    case MENU_ADICIONAR: menuAdicionar(); break;
    case MENU_RETIRAR: menuRetirar(); break;
    case MENU_TRANSFERIR_ORIGEM: menuTransferirOrigem(); break;
    case MENU_TRANSFERIR_DESTINO: menuTransferirDestino(); break;
    case MENU_CONFIRMACAO: menuConfirmacao(); break;
    case MENU_CONFIG_DINHEIRO: menuConfigDinheiro(); break;
    case MENU_CONFIG_SALARIO: menuConfigSalario(); break;
    case MENU_RESET_CONFIRM: menuResetConfirm(); break;
    case MENU_SALARIO: menuSalario(); break;
    case MENU_LISTA_JOGADORES: menuListaJogadores(); break;
    default: menuOp = MENU_INICIO; break;
  }
}

void atualizarAtividade() {
  timerInatividade.iniciar(TIMEOUT_INATIVIDADE_MS);
}

void resetarEstadoNovoJogo() {
  indiceCadastroCartao = 0;
  aguardandoExibicaoCadastro = false;
  auxSimulador = 1;
  transferenciaOrigem = -1;
  limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
  valorTransacao = 0;
  limparOperacaoPendente();
  limparJogadores();
}

void limparOperacaoPendente() {
  operacaoPendente = OP_NENHUMA;
  valorTransacao = 0;
  limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
}

void registrarOperacaoDesfazer(char op, int8_t j1, int8_t j2, long valor) {
  dadosDesfazer.operador = op;
  dadosDesfazer.posJog1 = j1;
  dadosDesfazer.posJog2 = j2;
  dadosDesfazer.valor = valor;
}

void exibirMensagemTemporaria(const char* msg, unsigned long ms, MenuEstado retorno) {
  copiarString(mensagemTemporaria, 17, msg);
  lcd.clear();
  lcd.print(mensagemTemporaria);
  timerMensagem.iniciar(ms);
  menuRetornoMsg = retorno;
  menuOp = MENU_MSG_TEMPORARIA;
}

void menuMsgTemporaria() {
  if (timerMensagem.expirou()) {
    timerMensagem.parar();
    if (mensagemTemporaria[0] != '\0') {
      lcd.clear();
    }
    limparBuffer(textoEmTela, 17);
    limparBuffer(mensagemTemporaria, 17);
    menuOp = menuRetornoMsg;
  }
}

void exibeLcd(int col, int lin, const char* texto) {
  if (!stringsIguais(textoEmTela, texto)) {
    lcd.clear();
    lcd.setCursor(col, lin);
    lcd.print(texto);
    copiarString(textoEmTela, 17, texto);
  }
}

void mostrarErroCartao() {
  exibirMensagemTemporaria("Cartao invalido", TEMPO_TELA_MS, MENU_CALCULADORA);
}

int procuraCartaoPorUid(const char* uid) {
  for (int i = 1; i < qtdCartoesCadastrados; i++) {
    if (stringsIguais(cartoes[i].codCartao, uid)) {
      return i;
    }
  }
  return -1;
}

int aproximaCartao() {
  exibeLcd(0, 0, "Aproxime cartao");

  if (MODO_SIMULADOR) {
    if (auxSimulador < qtdCartoesCadastrados) {
      return auxSimulador++;
    }
    auxSimulador = 1;
    return 1;
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return -1;
  }

  char tagID[18];
  limparBuffer(tagID, 18);
  int pos = 0;
  for (byte i = 0; i < mfrc522.uid.size && pos < 17; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10 && pos < 16) {
      tagID[pos++] = '0';
    }
    byte high = (mfrc522.uid.uidByte[i] >> 4) & 0x0F;
    byte low = mfrc522.uid.uidByte[i] & 0x0F;
    tagID[pos++] = high < 10 ? ('0' + high) : ('a' + high - 10);
    tagID[pos++] = low < 10 ? ('0' + low) : ('a' + low - 10);
  }
  tagID[pos] = '\0';

  mfrc522.PICC_HaltA();

  int idx = procuraCartaoPorUid(tagID);
  if (idx != -1) {
    return idx;
  }

  return cadastrarCartaoDinamico(tagID);
}

int procuraJogador(int codCartao) {
  for (int i = 0; i < qtdDeJogadores; i++) {
    if (players[i].id == codCartao) {
      return i;
    }
  }
  return -1;
}

void menuDeInicio() {
  exibeLcd(0, 0, "Novo jogo?");
  lcd.setCursor(0, 1);
  lcd.print("1-Sim 2-Nao #/*");

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla == '1') {
    resetarEstadoNovoJogo();
    exibirMensagemTemporaria("Vamos jogar...", TEMPO_TELA_MS, MENU_QTD_JOGADORES);
  } else if (tecla == '2') {
    if (carregarDaEEPROM() && qtdDeJogadores >= 2) {
      exibirMensagemTemporaria("Continuando...", TEMPO_TELA_MS, MENU_LISTA_JOGADORES);
      indiceListaJogadores = 0;
    } else {
      exibirMensagemTemporaria("Sem jogos salvos", TEMPO_TELA_MS, MENU_INICIO);
    }
  } else if (tecla == '#') {
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
    menuOp = MENU_CONFIG_DINHEIRO;
  } else if (tecla == '*') {
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
    menuOp = MENU_CONFIG_SALARIO;
  } else if (tecla == 'D') {
    menuOp = MENU_RESET_CONFIRM;
    lcd.clear();
    lcd.print("Apagar dados?");
    lcd.setCursor(0, 1);
    lcd.print("#-Sim  D-Nao");
  }
}

void menuResetConfirm() {
  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla == '#') {
    apagarEEPROM();
    resetarEstadoNovoJogo();
    exibirMensagemTemporaria("Dados apagados", TEMPO_TELA_MS, MENU_INICIO);
  } else if (tecla == 'D') {
    menuOp = MENU_INICIO;
    lcd.clear();
  }
}

void menuConfigDinheiro() {
  lcd.setCursor(0, 0);
  lcd.print("Dinheiro inicio:");
  lcd.setCursor(0, 1);
  lcd.print(valorTela);
  lcd.print(" + confirma");

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla >= '0' && tecla <= '9') {
    if (tamanhoString(valorTela) < MAX_DIGITOS_VALOR) {
      adicionarDigito(valorTela, MAX_DIGITOS_VALOR + 1, tecla);
    }
  } else if (tecla == 'D') {
    removerUltimoDigito(valorTela);
  } else if (tecla == '+') {
    long valor = stringParaLong(valorTela);
    if (valor > 0) {
      dinheiroInicial = valor;
    }
    exibirMensagemTemporaria("Salvo!", TEMPO_TELA_MS, MENU_INICIO);
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
  } else if (tecla == 'C') {
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
    menuOp = MENU_INICIO;
  }
}

void menuConfigSalario() {
  lcd.setCursor(0, 0);
  lcd.print("Salario inicio:");
  lcd.setCursor(0, 1);
  lcd.print(valorTela);
  lcd.print(" + confirma");

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla >= '0' && tecla <= '9') {
    if (tamanhoString(valorTela) < MAX_DIGITOS_VALOR) {
      adicionarDigito(valorTela, MAX_DIGITOS_VALOR + 1, tecla);
    }
  } else if (tecla == 'D') {
    removerUltimoDigito(valorTela);
  } else if (tecla == '+') {
    long valor = stringParaLong(valorTela);
    if (valor > 0) {
      salarioPassagem = valor;
    }
    exibirMensagemTemporaria("Salvo!", TEMPO_TELA_MS, MENU_INICIO);
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
  } else if (tecla == 'C') {
    limparBuffer(valorTela, MAX_DIGITOS_VALOR + 1);
    menuOp = MENU_INICIO;
  }
}

void menuQtdJogadores() {
  lcd.setCursor(0, 0);
  lcd.print("Qtd jogadores");
  lcd.setCursor(0, 1);
  lcd.print("de 2 a ");
  lcd.print(QTD_MAXIMA_JOGADORES);

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla >= '2' && tecla <= ('0' + QTD_MAXIMA_JOGADORES)) {
    qtdDeJogadores = tecla - '0';
    indiceCadastroCartao = 0;
    menuOp = MENU_ESPERANDO_CARTAO;
    lcd.clear();
  } else if (tecla >= '0' && tecla <= '9') {
    exibirMensagemTemporaria("Use 2 a 6", TEMPO_TELA_MS, MENU_QTD_JOGADORES);
  }
}

void menuEsperandoCartao() {
  if (indiceCadastroCartao >= qtdDeJogadores) {
    salvarNaEEPROM();
    indiceListaJogadores = 0;
    menuOp = MENU_LISTA_JOGADORES;
    return;
  }

  if (aguardandoExibicaoCadastro) {
    if (timerMensagem.expirou()) {
      aguardandoExibicaoCadastro = false;
      indiceCadastroCartao++;
      limparBuffer(textoEmTela, 17);
    }
    return;
  }

  int cartaoCod = aproximaCartao();
  if (cartaoCod == -1) return;

  if (procuraJogador(cartaoCod) != -1) {
    exibirMensagemTemporaria("Cartao ja usado", TEMPO_TELA_MS, MENU_ESPERANDO_CARTAO);
    return;
  }

  players[indiceCadastroCartao].id = cartaoCod;
  players[indiceCadastroCartao].saldo = dinheiroInicial;

  lcd.clear();
  lcd.print(cartoes[cartaoCod].nomeFantasia);
  lcd.setCursor(0, 1);
  lcd.print("R$ ");
  lcd.print(players[indiceCadastroCartao].saldo);

  timerMensagem.iniciar(TEMPO_TELA_MS);
  aguardandoExibicaoCadastro = true;
}

void menuListaJogadores() {
  if (indiceListaJogadores >= qtdDeJogadores) {
    lcd.clear();
    menuOp = MENU_CALCULADORA;
    return;
  }

  if (!timerMensagem.estaAtivo()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(indiceListaJogadores + 1);
    lcd.print("->");
    lcd.print(cartoes[players[indiceListaJogadores].id].nomeFantasia);
    lcd.setCursor(0, 1);
    lcd.print("R$ ");
    lcd.print(players[indiceListaJogadores].saldo);
    timerMensagem.iniciar(TEMPO_TELA_MS);
  }

  if (timerMensagem.expirou()) {
    timerMensagem.parar();
    indiceListaJogadores++;
  }
}

void menuCalculadora() {
  lcd.setCursor(0, 0);
  lcd.print("Valor: * lista");
  lcd.setCursor(0, 1);
  lcd.print(valorTela);

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla >= '0' && tecla <= '9') {
    if (tamanhoString(valorTela) < MAX_DIGITOS_VALOR) {
      adicionarDigito(valorTela, MAX_DIGITOS_VALOR + 1, tecla);
      valorTransacao = stringParaLong(valorTela);
    }
  } else if (tecla == 'D') {
    if (tamanhoString(valorTela) > 0) {
      removerUltimoDigito(valorTela);
      valorTransacao = stringParaLong(valorTela);
    } else if (dadosDesfazer.operador != '\0') {
      operacaoDesfazer();
      menuOp = MENU_CALCULADORA;
    }
  } else if (tecla == '+' && valorTransacao > 0) {
    operacaoPendente = OP_ADICIONAR;
    menuOp = MENU_CONFIRMACAO;
  } else if (tecla == '-' && valorTransacao > 0) {
    operacaoPendente = OP_RETIRAR;
    menuOp = MENU_CONFIRMACAO;
  } else if (tecla == 'C' && valorTransacao > 0) {
    operacaoPendente = OP_TRANSFERIR;
    menuOp = MENU_CONFIRMACAO;
  } else if (tecla == '*') {
    indiceListaJogadores = 0;
    menuOp = MENU_LISTA_JOGADORES;
  } else if (tecla == '#' && valorTransacao == 0) {
    operacaoPendente = OP_SALARIO;
    valorTransacao = salarioPassagem;
    menuOp = MENU_SALARIO;
  }
}

void menuConfirmacao() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (operacaoPendente == OP_ADICIONAR) {
    lcd.print("Adicionar R$");
  } else if (operacaoPendente == OP_RETIRAR) {
    lcd.print("Retirar R$");
  } else if (operacaoPendente == OP_TRANSFERIR) {
    lcd.print("Transferir R$");
  }
  lcd.setCursor(0, 1);
  lcd.print(valorTransacao);
  lcd.print(" # ok D canc");

  char tecla = keypad.getKey();
  if (tecla == NO_KEY) return;
  atualizarAtividade();

  if (tecla == '#') {
    if (operacaoPendente == OP_ADICIONAR) {
      menuOp = MENU_ADICIONAR;
    } else if (operacaoPendente == OP_RETIRAR) {
      menuOp = MENU_RETIRAR;
    } else if (operacaoPendente == OP_TRANSFERIR) {
      transferenciaOrigem = -1;
      menuOp = MENU_TRANSFERIR_ORIGEM;
    }
    limparBuffer(textoEmTela, 17);
  } else if (tecla == 'D' || tecla == 'C') {
    limparOperacaoPendente();
    menuOp = MENU_CALCULADORA;
    lcd.clear();
  }
}

void menuAdicionar() {
  int cartao = aproximaCartao();
  if (cartao == -1) return;

  int pos = procuraJogador(cartao);
  if (pos == -1) {
    mostrarErroCartao();
    return;
  }

  players[pos].saldo += valorTransacao;
  registrarOperacaoDesfazer('+', pos, -1, valorTransacao);
  registrarHistorico('+', pos, -1, valorTransacao);
  mostraNovoSaldo(pos);
  limparOperacaoPendente();
}

void menuRetirar() {
  int cartao = aproximaCartao();
  if (cartao == -1) return;

  int pos = procuraJogador(cartao);
  if (pos == -1) {
    mostrarErroCartao();
    return;
  }

  if (players[pos].saldo < valorTransacao) {
    exibirMensagemTemporaria("Saldo insufic.", TEMPO_TELA_MS, MENU_CALCULADORA);
    limparOperacaoPendente();
    return;
  }

  players[pos].saldo -= valorTransacao;
  registrarOperacaoDesfazer('-', pos, -1, valorTransacao);
  registrarHistorico('-', pos, -1, valorTransacao);
  mostraNovoSaldo(pos);
  limparOperacaoPendente();
}

void menuTransferirOrigem() {
  exibeLcd(0, 0, "Cartao origem");
  lcd.setCursor(0, 1);
  lcd.print("1o jogador");

  int cartao = aproximaCartao();
  if (cartao == -1) return;

  int pos = procuraJogador(cartao);
  if (pos == -1) {
    mostrarErroCartao();
    limparOperacaoPendente();
    return;
  }

  if (players[pos].saldo < valorTransacao) {
    exibirMensagemTemporaria("Saldo insufic.", TEMPO_TELA_MS, MENU_CALCULADORA);
    limparOperacaoPendente();
    return;
  }

  transferenciaOrigem = pos;
  menuOp = MENU_TRANSFERIR_DESTINO;
  limparBuffer(textoEmTela, 17);
}

void menuTransferirDestino() {
  exibeLcd(0, 0, "Cartao destino");
  lcd.setCursor(0, 1);
  lcd.print("2o jogador");

  int cartao = aproximaCartao();
  if (cartao == -1) return;

  int pos = procuraJogador(cartao);
  if (pos == -1) {
    mostrarErroCartao();
    return;
  }

  if (pos == transferenciaOrigem) {
    exibirMensagemTemporaria("Mesmo jogador!", TEMPO_TELA_MS, MENU_TRANSFERIR_DESTINO);
    return;
  }

  players[transferenciaOrigem].saldo -= valorTransacao;
  players[pos].saldo += valorTransacao;

  registrarOperacaoDesfazer('T', transferenciaOrigem, pos, valorTransacao);
  registrarHistorico('T', transferenciaOrigem, pos, valorTransacao);

  mostraNovoSaldo(pos);
  transferenciaOrigem = -1;
  limparOperacaoPendente();
}

void menuSalario() {
  exibeLcd(0, 0, "Salario inicio");
  lcd.setCursor(0, 1);
  lcd.print("R$ ");
  lcd.print(valorTransacao);

  int cartao = aproximaCartao();
  if (cartao == -1) return;

  int pos = procuraJogador(cartao);
  if (pos == -1) {
    mostrarErroCartao();
    limparOperacaoPendente();
    return;
  }

  players[pos].saldo += valorTransacao;
  registrarOperacaoDesfazer('S', pos, -1, valorTransacao);
  registrarHistorico('S', pos, -1, valorTransacao);
  mostraNovoSaldo(pos);
  limparOperacaoPendente();
}

void operacaoDesfazer() {
  if (dadosDesfazer.operador == '+') {
    players[dadosDesfazer.posJog1].saldo -= dadosDesfazer.valor;
  } else if (dadosDesfazer.operador == '-') {
    players[dadosDesfazer.posJog1].saldo += dadosDesfazer.valor;
  } else if (dadosDesfazer.operador == 'T') {
    players[dadosDesfazer.posJog1].saldo += dadosDesfazer.valor;
    players[dadosDesfazer.posJog2].saldo -= dadosDesfazer.valor;
  } else if (dadosDesfazer.operador == 'S') {
    players[dadosDesfazer.posJog1].saldo -= dadosDesfazer.valor;
  }

  salvarNaEEPROM();
  dadosDesfazer.operador = '\0';
  dadosDesfazer.posJog1 = -1;
  dadosDesfazer.posJog2 = -1;
  dadosDesfazer.valor = 0;

  exibirMensagemTemporaria("Desfeito!", TEMPO_TELA_MS, MENU_CALCULADORA);
}

void mostraNovoSaldo(int posicaoJogador) {
  salvarNaEEPROM();
  lcd.clear();
  lcd.print(cartoes[players[posicaoJogador].id].nomeFantasia);
  lcd.setCursor(0, 1);
  lcd.print("R$ ");
  lcd.print(players[posicaoJogador].saldo);
  timerMensagem.iniciar(TEMPO_TELA_MS);
  menuRetornoMsg = MENU_CALCULADORA;
  menuOp = MENU_MSG_TEMPORARIA;
}

void telaDeCarregamento() {
  lcd.home();
  for (int i = 0; i < 2; i++) {
    lcd.setCursor(0, i);
    for (int j = 0; j < 16; j++) {
      lcd.write(0);
      delay(30);
    }
  }

  lcd.leftToRight();
  lcd.clear();
}
