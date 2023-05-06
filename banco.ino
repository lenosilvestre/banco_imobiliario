#include <Keypad.h>
#include <LiquidCrystal.h>
#include <IRremote.h>
#include <Thread.h>
#include <ThreadController.h>.
#include <EEPROM.h>
#include <string.h>


//CONTROLE IR
// Definição do pino IR
const byte pinIR = 12;
IRrecv receiver(pinIR);//Criando instancia IR
//decode_results results; // declarando os resultados

// Definição dos pinos utilizados para o teclado matricial
const byte linhas = 4;
const byte colunas = 4;
byte linhaPinos[] = {8, 9, 10, 11};
byte colunaPinos[] = {A0, A1, A2, A3};

// caracteres que representa as teclas do teclado matricial
char teclas[] = {'1', '2', '3', '+', '4', '5', '6', '-', '7', '8', '9', 'C', '*', '0', '#', 'D'};
// Criação do objeto Keypad
Keypad keypad = Keypad(makeKeymap(teclas), linhaPinos, colunaPinos, linhas, colunas);

// Definição dos pinos utilizados para o display LCD
const byte rs = 7;
const byte enable = 6;
const byte d4 = 5;
const byte d5 = 4;
const byte d6 = 3;
const byte d7 = 2;

// Criação do objeto LiquidCrystal
LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);


//Thread configurações

ThreadController cpu;
ThreadController sensores;
//Thread leituraDoTeclado;
Thread menuInicial;
Thread qtdDeJogadoresMenu;
Thread txtEsperandoCartao;
Thread leituraDoControle; //depois mudar para RFID
Thread iniciaCalculadora;
Thread opAdicionar;
Thread opRetirar;
Thread opTransferir;
// Variavel responsavel por armazenar o valor a ser exibido no LCD
String valor = "";

//String teclaPress = "";

const float dinheiroInicial = 100;

int qtdDeJogadores = 0;

const int qtdMaximaDeJogadores = 6; //define o maximo de jogadores

int EEPROM_endereco = 0;


//ESTRUTURA COM OS VALORES DOS JOGADORES
struct contaJogadores {
  String  codCartao;
  int num;
  float saldoConta;
};

//VARIAVEL QUE ARMAZENA OS JOGADORES DENTRO DE UM ARRAY
struct contaJogadores players [qtdMaximaDeJogadores ];

//VARIAVEL QUE ARMAZENA OS VALORES LIDOS DA EEPROM
//contaJogadores lerEeprom[qtdMaximaDeJogadores - 1];


void setup() {



  // Inicializa o receptor IR
  receiver.enableIRIn();

  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.begin(16, 2);

  // Imprime a mensagem inicial no display LCD
  // lcd.print("Digite o valor:");
  Serial.begin(9600);

  //INICIANDO INSTANCIAS DAS THREAD
  // leituraDoTeclado.setInterval(50);
  // leituraDoTeclado.onRun(teclado);

  menuInicial.setInterval(10);
  menuInicial.onRun(menuDeInicio);

  qtdDeJogadoresMenu.setInterval(10);
  qtdDeJogadoresMenu.onRun(qtdJogadores);
  qtdDeJogadoresMenu.enabled = false;

  txtEsperandoCartao.setInterval(1000);
  txtEsperandoCartao.onRun(esperandoCartao);
  txtEsperandoCartao.enabled = false;

  leituraDoControle.setInterval(10);
  leituraDoControle.onRun(controleRemoto);
  leituraDoControle.enabled = false;

  iniciaCalculadora.setInterval(99);
  iniciaCalculadora.onRun(calculadora);
  iniciaCalculadora.enabled = false;

  opAdicionar.setInterval(500);
  opAdicionar.onRun(operacaoAdicionar);
  opAdicionar.enabled = false;

  opRetirar.setInterval(500);
  opRetirar.onRun(operacaoRetirar);
  opRetirar.enabled = false;


  opTransferir.setInterval(500);
  opTransferir.onRun(operacaoTransferir);
  opTransferir.enabled = false;


  cpu.add(&menuInicial);
  cpu.add(&qtdDeJogadoresMenu);
  cpu.add(&txtEsperandoCartao);
  cpu.add(&leituraDoControle);

  cpu.add(&iniciaCalculadora);
  cpu.add(&opAdicionar);
  cpu.add(&opRetirar);
  cpu.add(&opTransferir);
  //  cpu.add(&leituraDoTeclado);

}


void loop() {

  cpu.run();



}

//MENU INICIAL
void menuDeInicio() {

  lcd.setCursor(0, 0);
  lcd.print("Novo jogo?");

  lcd.setCursor(0, 1);
  lcd.print("1-Sim  2-Nao");

  char tecla = keypad.getKey();

  if (tecla == '1') {
    menuInicial.enabled = false;
    lcd.clear();
    exibeLcd(0, 0, "Vamos jogar...");
    delay(1000);
    lcd.clear();
    qtdDeJogadoresMenu.enabled = true;
  }

}

//MENU DE SELÇÃO PARA QUANTIDADE DE JOGADORES
void qtdJogadores() {

  exibeLcd(0, 0, "Qtd de jogadores");
  exibeLcd(0, 1, "de 2 a " + String(qtdMaximaDeJogadores));

  int tecla = keypad.getKey() - '0';

  if (tecla > 1 &&  tecla <= qtdMaximaDeJogadores) {

    qtdDeJogadoresMenu.enabled = false;
    qtdDeJogadores =  tecla;
    lcd.clear();
    txtEsperandoCartao.enabled = true;

  } else if (tecla > qtdMaximaDeJogadores ) {
    lcd.clear();
    exibeLcd(0, 0, "Selecione de 2 a ");
    exibeLcd(0, 1, String(qtdMaximaDeJogadores) + " jogadores        ");
    delay(1000);
    lcd.clear();
  }

}

int aux = 0;

//MENU QUE ADICIONA O CARTAO PARA CADA JOGADOR A LISTA []
void esperandoCartao() {
  lcd.clear();

  if ( aux == qtdDeJogadores) { //Quantidade de cartões lida = a quantidade maxima de jogadores

   // salvaNaEEPROM();

    //calculadora();
    printListaJogadores();
    lcd.clear();

    menuInicial.enabled = false;
    iniciaCalculadora.enabled = true;
    txtEsperandoCartao.enabled = false;
  } else {
    int cartao = aproximaCartao();

    if (cartao != -1 && !jogadorAdicionado(cartao)) {

      players[aux].codCartao = String(cartao); //codigo do cartão
      players[aux].num = aux + 1;
      players[aux].saldoConta = dinheiroInicial;
      lcd.clear();
      exibeLcd(0, 0, players[aux].codCartao + " " + String(players[aux].num));
      exibeLcd(0, 1, "R$ " + String(players[aux].saldoConta));

      aux++;
    }
  }

}

int aproximaCartao() {

  exibeLcd(0, 0, "Aproxime o ");
  exibeLcd(0, 1, "cartao " + String(aux + 1));

  if (receiver.decode()) {
    //Serial.println(receiver.decodedIRData.command);     // imprime o HEX Code
    int vlControle = receiver.decodedIRData.command;
    receiver.resume();
    return  vlControle;
    // Receive the next value
  } else {
    return -1;
  }

}


//GRAVANDO NA EEPROM
bool MEMORIA_ATUALIZADA = false;
void salvaNaEEPROM() {

  for (int i = 0; i < qtdDeJogadores ; i++) {
    int endereco = EEPROM_endereco + i * sizeof(contaJogadores);
    EEPROM.put(endereco, players[i]);

  }
  MEMORIA_ATUALIZADA = true;

}

//LER EEPROM
void lendoEPRROM() {
  if (MEMORIA_ATUALIZADA) {

    for (int i = 0; i < qtdDeJogadores ; i++) {

      int endereco = EEPROM_endereco + i * sizeof(contaJogadores);
      EEPROM.get(endereco, players[i]);

    }
    MEMORIA_ATUALIZADA = false;


  }
}

//Verifica se o cartão do jogador já foi adicionado na lista
boolean jogadorAdicionado(int cartao) {
  for (int i = 0 ; i < qtdDeJogadores ; i++) {
    if (players[i].codCartao.equals(String(cartao))) {
      return true;
    }
  }
  return  false;
}

//IMPRIME LISTA DE JOGADORES
void printListaJogadores() {
  // Serial.println("LISTA DE JOGADORES");
  lendoEPRROM();
  //Serial.println();
  delay(50);
  lcd.clear();
  for (int i = 0; i < qtdDeJogadores; i++) {
    String codCartao = players[i].codCartao;
    String num  = String(players[i].num);
    String saldo = String(players[i].saldoConta);

    //  Serial.println("Cod: " + codCartao + " " + num + "-> saldo: R$ " + saldo);
    exibeLcd(0, 0, "Jogador " + num + ":");
    exibeLcd(0, 1, "R$ " + saldo);
    delay(1000);

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

opRealizada dadosDesfazer = {"", -1, -1, 0 };

long value2 = 0;
String valorTela = "";

void calculadora() {
  String teclaPress = "";
  //leituraDoTeclado.enabled = true;
  receiver.resume();
  char key =  keypad.getKey();

  //ERRO AQUI NAO QUER VALIDAR A TECLA
  if (key >= '0' && key <= '9') {

    valorTela += key;
    lcd.setCursor(0, 1);
    lcd.print(valorTela);
    value2 = valorTela.toInt();
    key = "";
  } else if (key == '+' && value2 != 0 ) {
    // Define a operação
    valorTela = "";
    opAdicionar.enabled = true;

    iniciaCalculadora.enabled = false;
    key = "";

  } else if (key == '-'  && value2 != 0) {
    // Define a operação
    valorTela = "";
    opRetirar.enabled = true;
    iniciaCalculadora.enabled = false;
    key = "";
  }
  else if (key == '*' ) {
    // Define a operação
    valorTela = "";
    key = "";
    printListaJogadores();

  }
  else if (key == 'C' && value2 != 0) {
    // Define a operação
    valorTela = "";
    key = "";
    iniciaCalculadora.enabled = false;
    opTransferir.enabled = true;
    //operacaoTransferir();

  }
  else if (key == 'D' && !valorTela.equals("")) {
    key = "";
    lcd.setCursor(valorTela.length() - 1, 1);
    lcd.print(" ");
    valorTela = valorTela.substring(0, valorTela.length() - 1);
    dadosDesfazer = {"", -1, -1, 0 };

    value2 = valorTela.toInt();

  }
  else if (key == 'D' && valorTela.equals("") ) {
    // Define a operação
    valorTela = "";
    key = "";
    value2 = 0;
    //iniciaCalculadora.enabled = false;
    operacaoDesfazer();

  }

  else {

    lcd.setCursor(0, 0);
    lcd.print("Digite o valor:" );

  }

}

//OPERAÇÃO DE ADICIONAR DINHEIRO A CONTA
void operacaoAdicionar() {

  leituraDoControle.enabled = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime cartao");

  if (!valor.equals("")) {
    int posicaoJogador = procuraJogador();;

    if (posicaoJogador >= 0 && posicaoJogador <= qtdDeJogadores ) {

      players[posicaoJogador].saldoConta += value2;
      mostraNovoSaldo(posicaoJogador);

      dadosDesfazer = {"+", posicaoJogador, -1, value2 };
      value2 = 0;

      opAdicionar.enabled = false;

    }
  }




}

//OPERAÇÃO DE RETIRAR DINHEIRO A CONTA
void operacaoRetirar() {
  leituraDoControle.enabled = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime cartao");

  if (!valor.equals("")) {
    int posicaoJogador = procuraJogador();;

    if (posicaoJogador >= 0 && posicaoJogador <= qtdDeJogadores ) {

      players[posicaoJogador].saldoConta -= value2;
      mostraNovoSaldo(posicaoJogador);

      dadosDesfazer = {"-", posicaoJogador, -1, value2 };
      value2 = 0;
      opRetirar.enabled = false;



    }
  }

}

byte jogadorCont = 0;


void operacaoTransferir() {

  leituraDoControle.enabled = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime cartao");
  lcd.setCursor(0, 1);
  lcd.print(String(jogadorCont + 1) + "o Jogador");



  if (!valor.equals("")) {
    int posicaoJogador = procuraJogador();;

    if (posicaoJogador >= 0 && posicaoJogador <= qtdDeJogadores ) {
      if (jogadorCont == 0) {
        players[posicaoJogador].saldoConta -= value2;
        jogadorCont++;

        dadosDesfazer.posJog1 = posicaoJogador;

      } else if (jogadorCont == 1) {

        players[posicaoJogador].saldoConta += value2;
        mostraNovoSaldo(posicaoJogador);
        dadosDesfazer.posJog2 = posicaoJogador;
        dadosDesfazer.operador = "T";
        dadosDesfazer.valorTransferido = value2;

        value2 = 0;
        jogadorCont = 0;
        opTransferir.enabled = false;
        iniciaCalculadora.enabled = true;

      }
    }
  }
}
//desfaz a ultima ação



void operacaoDesfazer() {


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Desfazendo");



  if (!dadosDesfazer.operador.equals("")) {

    int posicaoJogador = dadosDesfazer.posJog1;


    if (dadosDesfazer.operador.equals("+")) {
      players[posicaoJogador].saldoConta -= dadosDesfazer.valorTransferido;
      mostraNovoSaldo(posicaoJogador);
      dadosDesfazer = {"", -1, -1, 0 };

    }
    else if (dadosDesfazer.operador.equals("-")) {
      players[posicaoJogador].saldoConta += dadosDesfazer.valorTransferido;
      mostraNovoSaldo(posicaoJogador);
      dadosDesfazer = {"", -1, -1, 0 };

    } else if (dadosDesfazer.operador.equals("T")) {
      // Serial.print("pos "+String(dadosDesfazer.posJog1)+ "\nPOs2 "+String(dadosDesfazer.posJog2) +" valor "+String(dadosDesfazer.valorTransferido));
      players[dadosDesfazer.posJog1].saldoConta += dadosDesfazer.valorTransferido;
      players[dadosDesfazer.posJog2].saldoConta -= dadosDesfazer.valorTransferido;
      mostraNovoSaldo(dadosDesfazer.posJog1);
      dadosDesfazer = {"", -1, -1, 0 };

    }
  }
}

//exibe no serial

//EXIBE NO LCD O NOVO SALDO E SALVA NA EEPROM O NOVO SALDO
void mostraNovoSaldo(int posicaoJogador) {
  salvaNaEEPROM();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saldo " + players[posicaoJogador].codCartao);
  lcd.setCursor(0, 1);
  lcd.print("R$ " + String(players[posicaoJogador].saldoConta));


  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Concluido");
  delay(500);
  leituraDoControle.enabled = false;
  iniciaCalculadora.enabled = true;

}


//RETORNA A POSIÇÃO DO JOGADOR NA LISTA
int procuraJogador() {

  //controleRemoto();

  for (int i = 0 ; i < qtdDeJogadores ; i++) {

    if (players[i].codCartao.equals(valor)) {
      // Serial.println("Encontrou");
      valor = "";
      return i;
    }
    //Serial.println("procurando...");
  }

  return -1;

}


//LEITURA DO TECLADO
void teclado() {
  char tecla = keypad.getKey();

  if (tecla != NO_KEY) {


    //     //  Serial.println(teclaPress);
  }

}

//FUNÇÃO PARA APAGAR UM DIGITO
void apagarCaractere() {
  lcd.setCursor(valor.length() - 1, 1);
  lcd.print(" ");
  valor = valor.substring(0, valor.length() - 1);
}

//EXIBE UMA MENSAGEM NO LCD COM PARAMENTROS INFORMADOS
void exibeLcd(int colunaLCD, int linhaLCD, String texto) {
  lcd.setCursor(colunaLCD, linhaLCD);
  lcd.print(texto);

}

//FUNÇÃO PARA LER O CONTROLE
void controleRemoto() {
  valor = "";
  // Checks received an IR signal
  if (receiver.decode()) {
    //Serial.println(receiver.decodedIRData.command);     // imprime o HEX Code
    valor = receiver.decodedIRData.command;
    receiver.resume();  // Receive the next value
  }

}

