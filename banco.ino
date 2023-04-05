#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Inclui a biblioteca para o display LCD
#include <Keypad.h>  // Inclui a biblioteca para o teclado matricial
#include <SPI.h>
#include <MFRC522.h>  // Inclui a biblioteca para o módulo RFID

// Define os pinos utilizados para o display LCD
#define LCD_ADDRESS 0x27  // Endereço I2C do display LCD
#define LCD_COLUMNS 16    // Número de colunas do display LCD
#define LCD_ROWS 2        // Número de linhas do display LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Define os pinos utilizados para o teclado matricial
const byte ROWS = 4;  // Número de linhas no teclado matricial
const byte COLS = 4;  // Número de colunas no teclado matricial
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};  // Pinos das linhas do teclado matricial
byte colPins[COLS] = {5, 4, 3, 2};  // Pinos das colunas do teclado matricial
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Define os pinos utilizados para o módulo RFID
#define SS_PIN 10  // Pino Slave Select
#define RST_PIN 9  // Pino de reset
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Cria um objeto MFRC522

void setup() {
  Serial.begin(9600);   // Inicializa a comunicação serial
  lcd.init();           // Inicializa o display LCD
  lcd.backlight();      // Liga a luz de fundo do display LCD
  lcd.clear();          // Limpa o display LCD
  SPI.begin();          // Inicializa a interface SPI
  mfrc522.PCD_Init();   // Inicializa o módulo RFID
}

void loop() {
  // Lê o teclado matricial para detectar qual tecla foi pressionada
  char key = keypad.getKey();
  
  if (key) {  // Se uma tecla foi pressionada
    if (key == 'A') {  // Se a tecla A foi pressionada, inicia a leitura do cartão RFID
      lcd.clear();  // Limpa o display LCD
      lcd.print("Aproxime o");
      lcd.setCursor(0, 1);
      lcd.print("cartao RFID...");
      
      // Aguarda até que um cartão seja detectado
      while (!mfrc522.PICC_IsNewCardPresent()) {
        delay(10);
      }
      
      // Lê o UID do cartão
      mfrc522.PICC_ReadCardSerial();
      String cardUID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
          cardUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    cardUID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  // Exibe o UID do cartão no display LCD
  lcd.clear();
  lcd.print("Cartao:");
  lcd.setCursor(0, 1);
  lcd.print(cardUID);
  
  // Aguarda 3 segundos antes de limpar o display LCD
  delay(3000);
  lcd.clear();
}
else {  // Se outra tecla foi pressionada, exibe a tecla no display LCD
  lcd.print(key);
}
