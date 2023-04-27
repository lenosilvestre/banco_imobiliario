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

// Matriz de caracteres que representa as teclas do teclado matricial
char teclas[linhas][colunas] = {
  {'1', '2', '3', '+'}, //acrescentar
  {'4', '5', '6', '-'}, //Subtrair
  {'7', '8', '9', 'C'}, //Transferir
  {'*', '0', '#', 'D'}  //Desfazer
};
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
Thread leituraDoTeclado;
Thread menuTextoLcd;
Thread qtdDeJogadoresMenu;
Thread txtEsperandoCartao;
Thread leituraDoControle; //depois mudar para RFID
Thread iniciaCalculadora;
Thread opAdicionar;
Thread opRetirar;
Thread opTransferir;
// Variavel responsavel por armazenar o valor a ser exibido no LCD
String valor = "";

String menu = "";
String teclaPress = "";

float dinheiroInicial = 15000;

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
struct contaJogadores players [qtdMaximaDeJogadores - 1];

//VARIAVEL QUE ARMAZENA OS VALORES LIDOS DA EEPROM
contaJogadores lerEeprom[qtdMaximaDeJogadores - 1];


void setup() {



  // Inicializa o receptor IR
  receiver.enableIRIn();

  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.begin(16, 2);

  // Imprime a mensagem inicial no display LCD
  // lcd.print("Digite o valor:");
  Serial.begin(9600);

  //INICIANDO INSTANCIAS DAS THREAD
  leituraDoTeclado.setInterval(50);
  leituraDoTeclado.onRun(teclado);

  menuTextoLcd.setInterval(1005);
  menuTextoLcd.onRun(menuDeInicio);

  qtdDeJogadoresMenu.setInterval(1000);
  qtdDeJogadoresMenu.onRun(qtdJogadores);
  qtdDeJogadoresMenu.enabled = false;

  txtEsperandoCartao.setInterval(1001);
  txtEsperandoCartao.onRun(esperandoCartao);
  txtEsperandoCartao.enabled = false;

  leituraDoControle.setInterval(1000);
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


  cpu.add(&menuTextoLcd);
  cpu.add(&qtdDeJogadoresMenu);
  cpu.add(&txtEsperandoCartao);
  cpu.add(&leituraDoControle);

  cpu.add(&iniciaCalculadora);
  cpu.add(&opAdicionar);
  cpu.add(&opRetirar);
  cpu.add(&opTransferir);
  cpu.add(&leituraDoTeclado);

}


void loop() {

  cpu.run();



}
String textoDeMenu [] = {"Vamos jogar..."};

//MENU INICIAL
void menuDeInicio() {

  lcd.setCursor(0, 0);
  lcd.print("Novo jogo?");

  lcd.setCursor(0, 1);
  lcd.print("1-Sim  2-Nao");


  if (teclaPress.equals("1")) {
    menuTextoLcd.enabled = false;
    lcd.clear();
    exibeLcd(0, 0, "Vamos jogar...");
    teclaPress = "";
    delay(1000);
    qtdDeJogadoresMenu.enabled = true;
  }

  else {
    menu = "";
  }

}

//MENU DE SELÇÃO PARA QUANTIDADE DE JOGADORES
void qtdJogadores() {
  lcd.clear();

  exibeLcd(0, 0, "Qtd de jogadores");
  exibeLcd(0, 1, "de 2 a " + String(qtdMaximaDeJogadores));


  if (teclaPress.toInt() > 1 && teclaPress.toInt() <= qtdMaximaDeJogadores) {
   // Serial.println(teclaPress + " jogadores selecionados");

    qtdDeJogadoresMenu.enabled = false;

    qtdDeJogadores =  teclaPress.toInt();;

    txtEsperandoCartao.enabled = true;
    teclaPress = "";
   

  }
  if (!teclaPress.equals("")) {
    exibeLcd(0, 0, "Selecione de 2 a ");
    exibeLcd(0, 1, String(qtdMaximaDeJogadores) + " jogadores        ");
    teclaPress = "";
    delay(1000);
  }


}

int aux = 0;

//MENU QUE ADICIONA O CARTAO PARA CADA JOGADOR A LISTA []
void esperandoCartao() {
  lcd.clear();

  if ( aux == qtdDeJogadores) { //Quantidade de cartões lida = a quantidade maxima de jogadores
    txtEsperandoCartao.enabled = false;
    qtdDeJogadoresMenu.enabled = false;
    leituraDoControle.enabled = false;
    salvaNaEEPROM();
    iniciaCalculadora.enabled = true;
    //calculadora();
    printListaJogadores();


  } else {
    exibeLcd(0, 0, "Aproxime o ");
    // exibeLcd(0, 1, "cartao");

    exibeLcd(0, 1, "cartao " + String(aux + 1));
    leituraDoControle.enabled = true;


    if (!valor.equals("") && !jogadorAdicionado()) {
      players[aux].codCartao = valor; //codigo do cartão
      players[aux].num = aux + 1;
      players[aux].saldoConta = dinheiroInicial;

      valor = "";

      lcd.clear();
      exibeLcd(0, 0, players[aux].codCartao + " " + String(players[aux].num));
      exibeLcd(0, 1, "R$ " + String(players[aux].saldoConta));

      aux++;
      delay(1000);
    }
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
      EEPROM.get(endereco, lerEeprom[i]);
     
    }
    MEMORIA_ATUALIZADA = false;
  }
}

//Verifica se o cartão do jogador já foi adicionado na lista
boolean jogadorAdicionado() {
  int cont = 0;
  for (int i = 0 ; i < qtdDeJogadores ; i++) {
    if (players[i].codCartao.equals(valor)) {
      valor = "";
      cont++;
    }
  }
  return cont > 0 ? true : false;
}


//IMPRIME LISTA DE JOGADORES
void printListaJogadores() {
  Serial.println("LISTA DE JOGADORES");
  lendoEPRROM();
  delay(50);
  for (int i = 0; i < qtdDeJogadores; i++) {
    String codCartao = lerEeprom[i].codCartao;
    String num  = String(lerEeprom[i].num);
    String saldo = String(lerEeprom[i].saldoConta);

    Serial.println("Cod: " + codCartao + " " + num + "-> saldo: R$ " + saldo);
    delay(50);

  }
}

//CALCULADORA
long value1 = 0;
long value2 = 0;
char operation;
String valorTela = "";

void calculadora() {

  leituraDoTeclado.enabled = true;
  receiver.resume();
  char key = teclaPress[0];

  //ERRO AQUI NAO QUER VALIDAR A TECLA
  if (key >= '0' && key <= '9') {

    valorTela += teclaPress;
    teclaPress = "";
    lcd.setCursor(0, 1);
    lcd.print(valorTela);
    value2 = valorTela.toInt();
    key = "";
  } else if (key == '+' ) {
    // Define a operação
    teclaPress = "";
    valorTela = "";
    opAdicionar.enabled = true;
    iniciaCalculadora.enabled = false;
    key = "";
  } else if (key == '-' ) {
    // Define a operação
    teclaPress = "";
    valorTela = "";
    opRetirar.enabled = true;
    iniciaCalculadora.enabled = false;
    key = "";
  }
  else if (key == '*' ) {
    // Define a operação
    teclaPress = "";
    valorTela = "";
    key = "";
    printListaJogadores();

  }
  else if (key == 'C' ) {
    // Define a operação
    teclaPress = "";
    valorTela = "";
    key = "";
    iniciaCalculadora.enabled = false;
    opTransferir.enabled = true;

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
      opRetirar.enabled = false;

    }
  }

}

int jogadorCont = 0;


void operacaoTransferir() {

  leituraDoControle.enabled = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime cartao");
  lcd.setCursor(0, 1);
  lcd.print(String(jogadorCont) + "o Jogador");

  if (!valor.equals("")) {
    int posicaoJogador = procuraJogador();;

    if (posicaoJogador >= 0 && posicaoJogador <= qtdDeJogadores ) {
      if (jogadorCont == 0) {
        players[posicaoJogador].saldoConta -= value2;
        jogadorCont++;

      } else if (jogadorCont == 1) {

        players[posicaoJogador].saldoConta += value2;


        mostraNovoSaldo(posicaoJogador);
        opTransferir.enabled = false;
        iniciaCalculadora.enabled = true;
      }


    }
  }


}




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

    //se a tecla A for precionada apga o ultimo caractere
    if (tecla == 'A') {
      apagarCaractere();
    } else {

      teclaPress = tecla;
      //  Serial.println(teclaPress);
    }
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

