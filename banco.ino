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
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
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
Thread iniciaCalculadora;

// Variavel responsavel por armazenar o valor a ser exibido no LCD
String valor = "";

String menu = "";
String teclaPress = "";

float dinheiroInicial = 15000;

int qtdDeJogadores = 0;

const int qtdMaximaDeJogadores = 6; //define o maximo de jogadores

int EEPROM_endereco = 0;

String listaDeJogadores [5];

struct contaJogadores {
  String  codCartao;
  int num;
  float saldoConta;
};


struct contaJogadores players [qtdMaximaDeJogadores - 1];


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

  iniciaCalculadora.setInterval(0);
  iniciaCalculadora.onRun(calculadora);
  iniciaCalculadora.enabled = false;



  cpu.add(&menuTextoLcd);
  cpu.add(&qtdDeJogadoresMenu);
  cpu.add(&txtEsperandoCartao);
  cpu.add(&leituraDoControle);
  cpu.add(&leituraDoTeclado);
  cpu.add(&iniciaCalculadora);
}


void loop() {
  //ler a tecla precionada e adiciona a variavel
  cpu.run();


  //     players[0] = {"Leno", 1, 15000};
  //   players[1] = {"joao", 2, 15000};
  //   Serial.println(players[0].codCartao);
  //   Serial.println(players[1].codCartao);
  // delay(50000);

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
    Serial.println(teclaPress + " jogadores selecionados");

    qtdDeJogadoresMenu.enabled = false;

    qtdDeJogadores =  teclaPress.toInt();;

    txtEsperandoCartao.enabled = true;
    teclaPress = "";
    //GRAVA NO EPROM O VALOR PADRÃO NA CONTA

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

    printListaJogadores();

    iniciaCalculadora.enabled = true;

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
      Serial.println("add lista de jogadores");
      delay(1000);
    }
  }

}
//GRAVANDO NA EEPROM
void salvaNaEEPROM() {
  for (int i = 0; i < qtdDeJogadores ; i++) {
    int endereco = EEPROM_endereco + i *sizeof(contaJogadores);
    EEPROM.put(endereco, players[i]);
  }

}

contaJogadores lerEeprom[qtdMaximaDeJogadores - 1];
//LER EEPROM
void lendoEPRROM() {

  for (int i = 0; i < qtdDeJogadores ; i++) {
    int endereco = EEPROM_endereco + i * sizeof(contaJogadores);
    EEPROM.get(endereco, lerEeprom[i]);
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
  for (int i = 0; i < qtdDeJogadores; i++) {
    String codCartao = lerEeprom[i].codCartao;
    String num  = String(lerEeprom[i].num);
    String saldo = String(lerEeprom[i].saldoConta);

     Serial.println(codCartao + " " + num + "-> saldo: R$ " + saldo);


  }
}

//CALCULADORA
int value1 = 0;
int value2 = 0;
char operation;

void calculadora(){

  char key = keypad.getKey();

  if (key != NO_KEY) {
    if (key >= '0' && key <= '9') {
      // Adiciona o dígito ao valor atual
      if (operation == 0) {
        value1 = value1 * 10 + (key - '0');
        lcd.setCursor(0, 0);
        lcd.print(value1);
      } else {
        value2 = value2 * 10 + (key - '0');
        lcd.setCursor(0, 1);
        lcd.print(value2);
      }
    } else if (key == '+' || key == '-') {
      // Define a operação
      operation = key;
      lcd.setCursor(0, 1);
      lcd.print(key);
    } else if (key == '#') {
      // Realiza o cálculo e exibe o resultado
      int result;
      if (operation == '+') {
        result = value1 + value2;
      } else if (operation == '-') {
        result = value1 - value2;
      }
      lcd.setCursor(0, 1);
      lcd.print("= ");
      lcd.print(result);
      delay(2000);
      lcd.clear();
      value1 = 0;
      value2 = 0;
      operation = 0;
    }
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
    Serial.println(receiver.decodedIRData.command);     // imprime o HEX Code
    valor = receiver.decodedIRData.command;
    receiver.resume();  // Receive the next value
  }

}
