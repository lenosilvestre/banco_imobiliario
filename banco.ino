#include <Keypad.h>

#include <LiquidCrystal.h>

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

String valor = "";

void setup() {


  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.begin(16, 2);

  // Imprime a mensagem inicial no display LCD
  lcd.print("Digite o valor:");
  Serial.begin(9600);
}

void loop() {
 
 char tecla = keypad.getKey();
  
  if(tecla != NO_KEY){
    if(tecla == 'A'){
      	lcd.setCursor(valor.length()-1, 1);
        lcd.print(" ");
     	valor = valor.substring(0,valor.length()-1); 
    }else{
    	valor += tecla;
	}
  }
  
  exibeLcd();
}
void exibeLcd(){
  		lcd.setCursor(0, 1);
        lcd.print(valor);
  		Serial.println(valor);
}
