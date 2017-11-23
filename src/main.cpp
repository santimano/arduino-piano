#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Arduino.h>

// ## constantes ##

// lcd
LiquidCrystal_I2C lcd(0x3f, 20, 4);

// ethernet
byte mac[] = { 0xFE, 0xAD, 0xFE, 0xAF, 0xFE, 0xED };
char server[] = "192.168.1.1";
IPAddress ip(192, 168, 1, 2);
EthernetClient client;

// modos
enum modo
{
  TEST,
  NORMAL,
  MANTENIMIENTO
} modo;

// sonidos
enum sonido
{
  PIANO,
  GUITARRA,
  ORGANO,
  EDM,
  CUSTOM_1,
  CUSTOM_2
} sonido;

// request http
enum request
{
  GET,
  POST
} request;

// notas
const int keys[12] = { 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22 };
const char notes[12] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B' };
const int threshold = 2;
bool touched[12];
const int ledNotas = 8;

// BUZZER
const int BUZZER = 4;
const int NOTE_BUZZER = 440;
const int DURATION_BUZZER = 400;

// ## funciones ##

// leer pin capacitivo
uint8_t readCapacitivePin(int pinToMeasure) {
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  byte bitmask;

  port = portOutputRegister(digitalPinToPort(pinToMeasure));
  ddr = portModeRegister(digitalPinToPort(pinToMeasure));
  bitmask = digitalPinToBitMask(pinToMeasure);
  pin = portInputRegister(digitalPinToPort(pinToMeasure));

  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);

  // Prevent the timer IRQ from disturbing our measurement
  noInterrupts();

  // Make the pin an input with the internal pull-up on
  *ddr &= ~(bitmask);
  *port |= bitmask;

  // Now see how long the pin to get pulled up. This manual unrolling of the loop
  // decreases the number of hardware cycles between each read of the pin,
  // thus increasing sensitivity.

  uint8_t cycles = 17;
       if (*pin & bitmask) { cycles =  0;}
  else if (*pin & bitmask) { cycles =  1;}
  else if (*pin & bitmask) { cycles =  2;}
  else if (*pin & bitmask) { cycles =  3;}
  else if (*pin & bitmask) { cycles =  4;}
  else if (*pin & bitmask) { cycles =  5;}
  else if (*pin & bitmask) { cycles =  6;}
  else if (*pin & bitmask) { cycles =  7;}
  else if (*pin & bitmask) { cycles =  8;}
  else if (*pin & bitmask) { cycles =  9;}
  else if (*pin & bitmask) { cycles = 10;}
  else if (*pin & bitmask) { cycles = 11;}
  else if (*pin & bitmask) { cycles = 12;}
  else if (*pin & bitmask) { cycles = 13;}
  else if (*pin & bitmask) { cycles = 14;}
  else if (*pin & bitmask) { cycles = 15;}
  else if (*pin & bitmask) { cycles = 16;}

  // End of timing-critical section
  interrupts();

  // Discharge the pin again by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;

  return cycles;
}

// manejar apretado pin
void handleKey(int index) {
  int cycles = readCapacitivePin(keys[index]);

  if (!touched[index] && cycles >= threshold) {
    touched[index] = true;
    Serial.write(notes[index]);
    digitalWrite(ledNotas,HIGH);
  }

  if (touched[index] && cycles < threshold) {
    digitalWrite(ledNotas,LOW);    
    touched[index] = false;
  }
}

// mandar un request http
void mandarRequestHttp(int tipoRequest, char url[], char parametro[]) {
  char buffer[128];

  if (client.connect(server, 3000)) {

    switch(tipoRequest) {
      case GET:
      sprintf(buffer,"GET %s HTTP/1.1", url);
      break;
    case POST:
      sprintf(buffer,"POST %s HTTP/1.1", url);
      break;
    }

    client.println(buffer);
    sprintf(buffer, "Host: %s", server);
    client.println(buffer);
    client.println("Connection: close");
    if (parametro != NULL) {
      client.println("Content-Type: application/x-www-form-urlencoded");
      sprintf(buffer,"Content-Length: %u\r\n", strlen(parametro));
      client.println(buffer);
      client.print(parametro);
    }
    client.println();
  } else {
    // TODO: mostrar error de conexion
  }

  delay(2000);
  
  while (client.available()) {
    char c = client.read();
    // Serial.print(c);
  }

  client.stop();
}

// cambiar el tipo de sonido
void cambiarTipoSonido() {
  char url[16];
  char modo[32];

  strcpy(url,"/mode");

  switch (sonido)
  {
  case PIANO:
    strcpy(modo, "modo=piano");
    break;
  case GUITARRA:
    strcpy(modo, "modo=acoustic");
    break;
  case ORGANO:
    strcpy(modo, "modo=organ");
    break;
  case EDM:
    strcpy(modo, "modo=edm");
    break;
  case CUSTOM_1:
    strcpy(modo, "modo=custom_1");
    break;
  case CUSTOM_2:
    strcpy(modo, "modo=custom_2");
    break;
  }
  
  mandarRequestHttp(POST, url, modo);
}

// imprimir el lcd
void imprimirLcd()
{
  switch (modo)
  {
  case TEST:
    lcd.clear();
    lcd.print("MODO: TEST");
    break;
  case NORMAL:
    lcd.clear();
    lcd.print("MODO: NORMAL");
    break;
  case MANTENIMIENTO:
    lcd.clear();
    lcd.print("MODO: MANT.");
    break;
  }

  switch (sonido)
  {
  case PIANO:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: PIANO");
    break;
  case GUITARRA:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: GUITARRA");
    break;
  case ORGANO:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: ORGANO");
    break;
  case EDM:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: EDM");
    break;
  case CUSTOM_1:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: CUSTOM_1");
    break;
  case CUSTOM_2:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: CUSTOM_2");
    break;
  }
}

// leer el bluetooth
void leerBluetooth()
{
  if (Serial1.available())
  {
    int lectura = Serial1.read();
    if (lectura == 'T'){
      modo = TEST;
    }
    else if (lectura == 'N') {
      modo = NORMAL;
    }
    else if (lectura == 'M'){
      modo = MANTENIMIENTO;
    }
    else if (lectura == 'P')
    {
      sonido = PIANO;
      cambiarTipoSonido();
    }
    else if (lectura == 'G')
    {
      sonido = GUITARRA;
      cambiarTipoSonido();
    }
    else if (lectura == 'O')
    {
      sonido = ORGANO;
      cambiarTipoSonido();
    }
    else if (lectura == 'E')
    {
      sonido = EDM;
      cambiarTipoSonido();
    }
    else if (lectura == '1' && modo == MANTENIMIENTO)
    {
      sonido = CUSTOM_1;
      cambiarTipoSonido();
    }
    else if (lectura == '2' && modo == MANTENIMIENTO)
    {
      sonido = CUSTOM_2;
      cambiarTipoSonido();
    }
    tone(BUZZER, NOTE_BUZZER, DURATION_BUZZER);
    imprimirLcd();
  }
}

// notas de prueba
void testNotes () {
  Serial.write(notes[0]);
  delay(400);

  Serial.write(notes[4]);
  delay(400);

  Serial.write(notes[7]);
  delay(400);
}

// probar leds
void testLED () {
  lcd.clear();
  lcd.print("MODO: TEST");
  lcd.setCursor(0, 1);
  lcd.print("TEST: LEDS");

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(ledNotas, HIGH);

  delay(2000);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ledNotas, LOW);
}

// probar ethernet
void testEthernet () {
  lcd.clear();
  lcd.print("MODO: TEST");
  lcd.setCursor(0, 1);
  lcd.print("TEST: ETHERNET");

  char url[16];
  strcpy(url,"/test");
  mandarRequestHttp(POST, url, NULL);
}

// probar los sonidos
void testSonidos () {
  lcd.clear();
  lcd.print("MODO: TEST");
  lcd.setCursor(0, 1);
  lcd.print("TEST: SONIDOS");

  sonido = GUITARRA;
  cambiarTipoSonido();
  testNotes();

  sonido = ORGANO;
  cambiarTipoSonido();
  testNotes();

  sonido = EDM;
  cambiarTipoSonido();
  testNotes();

  sonido = PIANO;
  cambiarTipoSonido();
  testNotes();
  
}

void testBuzzer ()
{
  tone(BUZZER, 261, 400);
  delay(400);
  tone(BUZZER, 329, 400);
  delay(400);
  tone(BUZZER, 391, 400);
  delay(400);
  tone(BUZZER, 523, 400);
  delay(400);
}

// ## setup ##
void setup()
{
  // BUZZER
  pinMode(BUZZER, OUTPUT);

  // LED Arduino
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledNotas, OUTPUT);

  // serial: datos
  Serial.begin(115200);

  // serial1: bluetooth
  Serial1.begin(9600);

  Ethernet.begin(mac, ip);
  
  delay(1000);

  // init lcd
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Bienvenido!");
  delay(2000);

  // init modo
  modo = NORMAL;
  sonido = PIANO;

  imprimirLcd();
  tone(BUZZER, NOTE_BUZZER, DURATION_BUZZER);  
}

// # loop ## 
void loop()
{

  leerBluetooth();

  switch (modo)
  {
  case TEST:
    testBuzzer();
    testLED();
    testEthernet();
    testSonidos();
    break;
  default:
    for (int i = 0; i < 12; i++) {
      handleKey(i);
    }  
    break;
  }

  delay(10);
}
