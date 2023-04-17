#include <Keypad.h>
#include <LiquidCrystal.h>
#include <IRremote.h>
#include <Thread.h>
#include <ThreadController.h>.


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
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
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

// Variavel responsavel por armazenar o valor a ser exibido no LCD
String valor = "";

String menu = "";
String teclaPress = "";
int contaJogador1 = 0;
int contaJogador2 = 0;
int contaJogador3 = 0;
int contaJogador4 = 0;
int valorPadrao = 15000;
int qtdDeJogadores = 0;

String listaDeJogadores [5];


void setup() {

  // Inicializa o receptor IR
  receiver.enableIRIn();

  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.begin(16, 2);

  // Imprime a mensagem inicial no display LCD
  // lcd.print("Digite o valor:");
  Serial.begin(9600);

  //INICIANDO INSTANCIAS DAS THREAD
  leituraDoTeclado.setInterval(10);
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



  cpu.add(&menuTextoLcd);
  cpu.add(&qtdDeJogadoresMenu);
  cpu.add(&txtEsperandoCartao);
  cpu.add(&leituraDoControle);
  cpu.add(&leituraDoTeclado);
}


void loop() {
  //ler a tecla precionada e adiciona a variavel
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
  exibeLcd(0, 1, "de 2 a 4?");
  int contaJogador1 = valorPadrao;

  if (teclaPress.equals("1")) {
    exibeLcd(0, 0, "Selecione de 2 a ");
    exibeLcd(0, 1, "4 jogadores        ");
    teclaPress = "";
    delay(2000);
    // qtdDeJogadoresMenu.enabled = false;
    //GRAVA NO EPROM O VALOR PADRÃO NA CONTA

  }  if (teclaPress.equals("2")) {
    Serial.println("2 jogadores selecionados");
    teclaPress = "";
    qtdDeJogadoresMenu.enabled = false;
    //GRAVA NO EPROM O VALOR PADRÃO NA CONTA
    qtdDeJogadores = 2;
    int contaJogador2 = valorPadrao;
    txtEsperandoCartao.enabled = true;
  }  if (teclaPress.equals("3")) {
    Serial.println("3 jogadores selecionados");
    teclaPress = "";
    qtdDeJogadoresMenu.enabled = false;
    //GRAVA NO EPROM O VALOR PADRÃO NA CONTA
    qtdDeJogadores = 3;
    int contaJogador3 = valorPadrao;
    txtEsperandoCartao.enabled = true;
  }  if (teclaPress.equals("4")) {
    Serial.println("4 jogadores selecionados");
    teclaPress = "";
    qtdDeJogadoresMenu.enabled = false;
    //GRAVA NO EPROM O VALOR PADRÃO NA CONTA
    qtdDeJogadores = 4;
    int contaJogador4 = valorPadrao;
    txtEsperandoCartao.enabled = true;
  } 
  
  if (!teclaPress.equals("")) {
    exibeLcd(0, 0, "Selecione de 2 a ");
    exibeLcd(0, 1, "4 jogadores        ");
    teclaPress = "";
    delay(1000);
  }


}
int aux = 0;


//MENU QUE ADICIONA O CARTAO PARA CADA JOGADOR A LISTA []
void esperandoCartao() {
  lcd.clear();

  if ( aux == qtdDeJogadores) {
    txtEsperandoCartao.enabled = false;
    qtdDeJogadoresMenu.enabled = false;
    leituraDoControle.enabled = false;
    printListaJogadores();

  } else {
    exibeLcd(0, 0, "Aproxime o ");
    // exibeLcd(0, 1, "cartao");

    exibeLcd(0, 1, "cartao " + String(aux + 1));
    leituraDoControle.enabled = true;


    if (!valor.equals("") && !jogadorAdicionado()) {
      listaDeJogadores[aux] = valor;
      aux++;
      Serial.println("add lista de jogadores");
    }
  }

}

//Verifica se o cartão do jogador já foi adicionado na lista
boolean jogadorAdicionado() {
  int cont = 0;
  for (int i = 0 ; i < qtdDeJogadores ; i++) {
    if (listaDeJogadores[i].equals(valor)) {
      valor = "";
      cont++;
    }
  }
  return cont > 0 ? true : false;
}




//IMPRIME LISTA DE JOGADORES
void printListaJogadores() {
  Serial.println("LISTA DE JOGADORES");
  for (int i = 0; i < qtdDeJogadores; i++) {
    Serial.println(" jogador -> " + String(i + 1) + " - " + listaDeJogadores[i]);
  }
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
      Serial.println(teclaPress);
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
    // Serial.println(receiver.decodedIRData.command);     // imprime o HEX Code
    valor = receiver.decodedIRData.command;
    receiver.resume();  // Receive the next value
  }

}
