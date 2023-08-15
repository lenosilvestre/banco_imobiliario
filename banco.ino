#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <SPI.h>

//CONTROLE IR
// Definição do pino IR
const byte pinIR = 13;
IRrecv receiver(pinIR);  //Criando instancia IR


//DEFINIÇÃO DOS PINO RFID <<<<<<<<<<
const byte SS_PIN = 10;
const byte RST_PIN = 9;

MFRC522 mfrc522(SS_PIN, RST_PIN);


// Definição dos pinos utilizados para o teclado matricial
const byte linhas = 4;
const byte colunas = 4;
byte linhaPinos[] = { 2, 3, 4, 5 };
byte colunaPinos[] = { A0, A1, A2, A3 };

// caracteres que representa as teclas do teclado matricial
char teclas[] = { '1', '2', '3', '+', '4', '5', '6', '-', '7', '8', '9', 'C', '*', '0', '#', 'D' };
// Criação do objeto Keypad
Keypad keypad = Keypad(makeKeymap(teclas), linhaPinos, colunaPinos, linhas, colunas);

// Definição dos pinos utilizados para o display LCD
// Criação do objeto LiquidCrystal para ser usada com o I2C
#define enderecoI2C 0x27  // Serve para definir o endereço do display.

LiquidCrystal_I2C lcd(enderecoI2C, 16, 2);

// Criação do objeto LiquidCrystal
//LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);


// Variavel responsavel por armazenar o valor a ser exibido no LCD
String valorTela = "";
//variavel que armazena o valor da transação
long value2 = 0;

float dinheiroInicial = 100;

int qtdDeJogadores = 0;

const int qtdMaximaDeJogadores = 6;  //define o maximo de jogadores

int EEPROM_endereco = 10;

bool MEMORIA_ATUALIZADA = false;  //verificador se houve atualização na lista de jogadores


//NOMEIA OS CARTÃO PARA FACILITAR A INDENTIFICAÇÃO DO USUARIO
struct nomeCartoes {
  String codCartao;
  String nomeFantasia;
};
struct nomeCartoes nomeCartao[5] = {
  { "NULNUL", "NULNUL" }, { "a7848236", "branco" }, { "b9d0d693", "azul" }, { "040a71b2dc4c81", "visa" }, { "040271b2dc4c81", "master" }
};

//ESTRUTURA COM OS VALORES DOS JOGADORES
struct contaJogadores {
  int num;
  float saldoConta;
};

//VARIAVEL QUE ARMAZENA OS JOGADORES DENTRO DE UM ARRAY
struct contaJogadores players[qtdMaximaDeJogadores];


//VARIAVEL QUE ARMAZENA OS VALORES DE TESTE
struct contaJogadores plTeste[qtdMaximaDeJogadores];

//CARACTERE ESPECIAL 'á'
byte aAcentuado[8] = {
  B00001,
  B00010,
  B01111,
  B00001,
  B01111,
  B10001,
  B01111,
  B00000
};



void setup() {

  // Inicializa o receptor IR
  receiver.enableIRIn();


  SPI.begin();
  //Inicialia RFID
  mfrc522.PCD_Init();


  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.init();                     // Serve para iniciar a comunicação com o display já conectado
  lcd.backlight();                // Serve para ligar a luz do display
  lcd.createChar(1, aAcentuado);  //caractere especial 'á'

  Serial.begin(9600);
}
int menuOp = 0;

void loop() {

  switch (menuOp) {
    case 0:
      menuDeInicio();
      break;
    case 1:
      qtdJogadores();
      break;
    case 2:
      esperandoCartao();
      break;
    case 3:
      calculadora();
      break;
    case 4:
      operacaoAdicionar();
      break;
    case 5:
      operacaoRetirar();
      break;
    case 6:
      operacaoTransferir();
      break;
  }
}


//MENU INICIAL
void menuDeInicio() {

  lcd.setCursor(0, 0);
  lcd.print("Novo jogo?");

  lcd.setCursor(0, 1);
  lcd.print("1-Sim  2-Nao");

  char tecla = keypad.getKey();

  if (tecla == '1') {

    lcd.clear();
    exibeLcd(0, 0, "Vamos jogar...");
    delay(500);
    lcd.clear();
    menuOp = 1;


  } else if (tecla == '2') {


    EEPROM.get(0, qtdDeJogadores);

    if (qtdDeJogadores > 1) {
      lcd.clear();
      exibeLcd(0, 0, "Continuando...");
      delay(1000);
      MEMORIA_ATUALIZADA = true;
      printListaJogadores();
      delay(500);

      menuOp = 3;


    } else {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("nao h");
      lcd.write(1);  //caractere especial 'á'
      lcd.print(" jogos");
      lcd.setCursor(0, 1);
      lcd.print("salvos");
      delay(1000);
      lcd.clear();
    }

  } else if (tecla == '#') {
    char key = keypad.getKey();
    lcd.clear();
    while (key != '+') {
      lcd.setCursor(0, 0);
      lcd.print("valor inicio:");
      key = keypad.getKey();
      if (key >= '0' && key <= '9') {

        valorTela += key;
        lcd.setCursor(0, 1);
        lcd.print(valorTela);
        dinheiroInicial = valorTela.toInt();
        key = "";
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("Dinheiro inicial");
    lcd.setCursor(0, 1);
    lcd.print(dinheiroInicial);
    delay(1500);
    valorTela = "";
    lcd.clear();
  }
}


//MENU DE SELÇÃO PARA QUANTIDADE DE JOGADORES
void qtdJogadores() {
  lcd.setCursor(0, 0);
  lcd.print("Qtd de jogadores");
  lcd.setCursor(0, 1);
  lcd.print("de 2 a ");
  lcd.print(qtdMaximaDeJogadores);

  int tecla = keypad.getKey() - '0';

  if (tecla > 1 && tecla <= qtdMaximaDeJogadores) {

    qtdDeJogadores = tecla;
    EEPROM.put(0, qtdDeJogadores);
    lcd.clear();
    menuOp = 2;

  } else if (tecla > qtdMaximaDeJogadores) {
    lcd.clear();

    lcd.print("Selecione de 2 a ");
    lcd.setCursor(0, 1);
    lcd.print(qtdMaximaDeJogadores);

    lcd.print(" jogadores");
    delay(1500);
    lcd.clear();
  }
}

int aux = 0;
//MENU QUE ADICIONA O CARTAO PARA CADA JOGADOR A LISTA []
void esperandoCartao() {

  if (aux == qtdDeJogadores) {  //Quantidade de cartões lida = a quantidade maxima de jogadores

    salvaNaEEPROM();

    printListaJogadores();
    lcd.clear();
    menuOp = 3;

  } else {


    int cartaoCod = aproximaCartao();


    if (cartaoCod != -1 && procuraJogador(cartaoCod) == -1) {

      // players[aux].codCartao = nomeCartao[cartaoCod].nomeFantasia;  //codigo do cartão
      players[aux].num = cartaoCod;  //Posição do nome fantasia no array
      players[aux].saldoConta = dinheiroInicial;

      lcd.clear();

      lcd.print(nomeCartao[cartaoCod].nomeFantasia);
      lcd.setCursor(0, 1);
      lcd.print("R$ " + String(players[aux].saldoConta));
      delay(1500);
      exibeLcd(0, 0, "Aproxime cartao ");
      aux++;
    }
  }
}
//int teste = 1 ;

int aproximaCartao() {

  exibeLcd(0, 0, "Aproxime cartao");


  // Verifica se há uma nova tag RFID presente
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Lê o ID da tag mfrc522
    String tagID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      tagID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    mfrc522.PICC_HaltA();  // Pára a leitura da tag atual

    for (int i = 0; i < sizeof(nomeCartao) / sizeof(nomeCartao[0]); i++) {
      if (nomeCartao[i].codCartao.equals(tagID)) {
        return i;
      }
    }
  }
  return -1;



  /*   if (teste == 1) {
    teste++;
    return 1;
  } if (teste == 2) {
    teste = 1;
    return 2;
  } */
}


//RETORNA A POSIÇÃO DO JOGADOR NA LISTA se já existe o jogador, retorna a posição
int procuraJogador(int codCartao) {

  for (int i = 0; i < qtdDeJogadores; i++) {

    if (players[i].num == codCartao) {
      return i;
    }
  }
  return -1;
}


//GRAVANDO NA EEPROM
void salvaNaEEPROM() {

  for (int i = 0; i < qtdDeJogadores; i++) {
    int endereco = EEPROM_endereco + i * sizeof(contaJogadores);

    EEPROM.put(endereco, players[i]);
  }
  MEMORIA_ATUALIZADA = true;
}

//LER EEPROM
void lendoEPRROM() {
  if (MEMORIA_ATUALIZADA) {

    for (int i = 0; i < qtdDeJogadores; i++) {

      int endereco = EEPROM_endereco + i * sizeof(contaJogadores);
      EEPROM.get(endereco, players[i]);

      int id = players[i].num;
    }
    MEMORIA_ATUALIZADA = false;
  }
  salvaNaEEPROM();
}




//IMPRIME LISTA DE JOGADORES
void printListaJogadores() {
  // Serial.println("LISTA DE JOGADORES");
  lendoEPRROM();
  //Serial.println(qtdDeJogadores);
  delay(50);
  lcd.clear();


  for (int i = 0; i < qtdDeJogadores; i++) {
    /*      Serial.print(" Ordem ");
      Serial.print(String(players[i].num));
      Serial.print(" Cod: ");
      Serial.print(players[i].codCartao  );
      Serial.print(" -> saldo: R$ ");
      Serial.println( String(players[i].saldoConta));
    */

    lcd.setCursor(0, 0);
    lcd.print(String(i + 1));
    lcd.setCursor(2, 0);
    lcd.print("->");
    lcd.setCursor(5, 0);
    lcd.print(nomeCartao[players[i].num].nomeFantasia);
    lcd.setCursor(0, 1);
    lcd.print("R$ ");
    lcd.setCursor(3, 1);
    lcd.print(String(players[i].saldoConta));
    delay(500);
    lcd.clear();
  }
  lcd.clear();
}

//CALCULADORA
struct opRealizada {
  String operador;
  int posJog1;
  int posJog2;
  long valorTransferido;
};

//Dados para gravados para poder desfazer a ultima operação
opRealizada dadosDesfazer = { "", -1, -1, 0 };

//Função responsavel por receber os valores e definir as operaçoes da maquina via teclado
void calculadora() {

  // receiver.resume();
  //leituraDoTeclado.enabled = true;

  char key = keypad.getKey();

  //ERRO AQUI NAO QUER VALIDAR A TECLA
  if (key >= '0' && key <= '9') {

    valorTela += key;
    lcd.setCursor(0, 1);
    lcd.print(valorTela);
    value2 = valorTela.toInt();
    key = "";
  } else if (key == '+' && value2 != 0) {
    exibeLcd(0, 0, "Aproxime cartao ");
    menuOp = 4;
    valorTela = "";
    key = "";

  } else if (key == '-' && value2 != 0) {
    exibeLcd(0, 0, "Aproxime cartao ");
    menuOp = 5;
    key = "";
    valorTela = "";



  } else if (key == '*') {
    // Define a operação Imprimir todos os jogadores e o saldo
    valorTela = "";
    key = "";
    printListaJogadores();
    menuOp = 3;

  } else if (key == 'C' && value2 != 0) {
    //Operação de transferir
    exibeLcd(0, 0, "Aproxime cartao ");
    menuOp = 6;
    valorTela = "";
    key = "";

  } else if (key == 'D' && !valorTela.equals("")) {
    // Define a operação apagar um digito da tela
    key = "";
    lcd.setCursor(valorTela.length() - 1, 1);
    lcd.print(" ");
    valorTela = valorTela.substring(0, valorTela.length() - 1);
    dadosDesfazer = { "", -1, -1, 0 };

    value2 = valorTela.toInt();

  } else if (key == 'D' && valorTela.equals("")) {
    // Define a operação desfazer a ultima operação caso não tenha digitos na tela
    valorTela = "";
    key = "";
    value2 = 0;

    if (!dadosDesfazer.operador.equals("")) {
      operacaoDesfazer();
    }

  }

  else if (value2 == 0) {

    lcd.setCursor(0, 0);
    lcd.print("Digite o valor:");
  }
}

//OPERAÇÃO DE ADICIONAR DINHEIRO A CONTA
void operacaoAdicionar() {

  int posicaoJogador = -1;
  int cartao = aproximaCartao();

  if (cartao != -1) {
    posicaoJogador = procuraJogador(cartao);
    Serial.print("id ");
    Serial.println(cartao);

    delay(100);
  }


  if (cartao != -1 && posicaoJogador != -1) {

    players[posicaoJogador].saldoConta += value2;
    dadosDesfazer = { "+", posicaoJogador, -1, value2 };
    value2 = 0;
    mostraNovoSaldo(posicaoJogador);
    menuOp = 3;
  }
}

//OPERAÇÃO DE RETIRAR DINHEIRO A CONTA
void operacaoRetirar() {

  int cartao = -1;
  cartao = aproximaCartao();

  int posicaoJogador = procuraJogador(cartao);

  if (cartao != -1 && posicaoJogador != -1) {

    if (players[posicaoJogador].saldoConta >= value2) {

      players[posicaoJogador].saldoConta -= value2;
      mostraNovoSaldo(posicaoJogador);

      dadosDesfazer = { "-", posicaoJogador, -1, value2 };
      value2 = 0;
      menuOp = 3;

    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Saldo insuficiente");
      value2 = 0;
      delay(1500);
      mostraNovoSaldo(posicaoJogador);
      menuOp = 3;
    }
  }
}

int jogadorCont = 0;


void operacaoTransferir() {


  int cartao = -1;
  cartao = aproximaCartao();

  lcd.setCursor(0, 1);
  lcd.print(String(jogadorCont + 1) + "o Jogador");

  int posicaoJogador = procuraJogador(cartao);

  if (cartao != -1 && posicaoJogador != -1) {

    if (jogadorCont == 0) {

      players[posicaoJogador].saldoConta -= value2;
      jogadorCont++;

      dadosDesfazer.posJog1 = posicaoJogador;
      delay(100);
      operacaoTransferir();

    } else if (jogadorCont == 1 && dadosDesfazer.posJog1 != posicaoJogador) {

      players[posicaoJogador].saldoConta += value2;
      mostraNovoSaldo(posicaoJogador);
      dadosDesfazer.posJog2 = posicaoJogador;
      dadosDesfazer.operador = "T";
      dadosDesfazer.valorTransferido = value2;

      value2 = 0;
      jogadorCont = 0;
      menuOp = 3;
    }
  }
}
//desfaz a ultima ação
void operacaoDesfazer() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Desfazendo");
  delay(1000);


  int posicaoJogador = dadosDesfazer.posJog1;


  if (dadosDesfazer.operador.equals("+")) {
    players[posicaoJogador].saldoConta -= dadosDesfazer.valorTransferido;
    mostraNovoSaldo(posicaoJogador);
    dadosDesfazer = { "", -1, -1, 0 };

  } else if (dadosDesfazer.operador.equals("-")) {
    players[posicaoJogador].saldoConta += dadosDesfazer.valorTransferido;
    mostraNovoSaldo(posicaoJogador);
    dadosDesfazer = { "", -1, -1, 0 };

  } else if (dadosDesfazer.operador.equals("T")) {

    players[dadosDesfazer.posJog1].saldoConta += dadosDesfazer.valorTransferido;
    players[dadosDesfazer.posJog2].saldoConta -= dadosDesfazer.valorTransferido;
    mostraNovoSaldo(dadosDesfazer.posJog1);
    dadosDesfazer = { "", -1, -1, 0 };
  }
}

//EXIBE NO LCD O NOVO SALDO E SALVA NA EEPROM O NOVO SALDO
void mostraNovoSaldo(int posicaoJogador) {
  salvaNaEEPROM();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(nomeCartao[players[posicaoJogador].num].nomeFantasia);
  lcd.setCursor(0, 1);
  lcd.print(String(players[posicaoJogador].saldoConta));
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Concluido");
  delay(1000);
  lcd.clear();
}

String textoEmTela = "";

//EXIBE UMA MENSAGEM NO LCD COM PARAMENTROS INFORMADOS
void exibeLcd(int colunaLCD, int linhaLCD, String texto) {
  if (!textoEmTela.equals(texto)) {
    lcd.clear();
    lcd.setCursor(colunaLCD, linhaLCD);
    lcd.print(texto);
    textoEmTela = texto;
  }
}
