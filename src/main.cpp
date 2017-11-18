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
char server[] = "192.168.3.57";
IPAddress ip(192, 168, 3, 58);
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
  BAJO
} sonido;

// request
enum request
{
  GET,
  POST
} request;

// notas
const int keys[12] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44 };
const char notes[12][6] = { "C4, ", "C#4, ", "D4, ", "D#4, ", "E4, ", "F4, ", "F#4, ", "G4, ", "G#4, ", "A4, ", "A#4, ", "B4, " };
const int threshold = 2;
bool touched[12];

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

  if (cycles >= threshold) {
    touched[index] = true;
    //Serial.print(index);
  }

  if (touched[index] && cycles < threshold) {
    touched[index] = false;
  }
}

// mandar un request http
int mandarRequestHttp(int tipoRequest, char parametro[]) 
{

  char request[1024];
  switch(tipoRequest) {
    case GET:
    strcpy(request,"GET ");
    break;
  case POST:
    strcpy(request,"POST ");
    break;
  }

  strcat(request, parametro);
  strcat(request," HTTP/1.1");

  if (client.connect(server, 8000)) {
    Serial.println("connected");
    client.println(request);
    client.println("Host: 192.168.3.57:8000");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
  }

  delay(2000);
  
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  return 1;
}

// cambiar el tipo de sonido
void cambiarTipoSonido()
{

  char tipoSonido[32];
  switch (sonido)
  {
  case PIANO:
    strcpy(tipoSonido, "/.vscode/arduino.json");
    break;
  case BAJO:
    strcpy(tipoSonido, "/.vscode/c_cpp_properties.json");
    break;
  }
  
  mandarRequestHttp(GET,tipoSonido);
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
  case BAJO:
    lcd.setCursor(0, 1);
    lcd.print("SONIDO: BAJO");
    break;
  }
}

// leer el bluetooth
void leerBluetooth()
{
  if (Serial1.available())
  {
    int lectura = Serial1.read();
    if (lectura == 'T')
      modo = TEST;
    else if (lectura == 'N')
      modo = NORMAL;
    else if (lectura == 'M')
      modo = MANTENIMIENTO;
    else if (lectura == 'P')
    {
      Serial.println("cambio a piano");
      sonido = PIANO;
      cambiarTipoSonido();
    }
    else if (lectura == 'B')
    {
      Serial.println("cambio a bajo");
      sonido = BAJO;
      cambiarTipoSonido();
    }

    imprimirLcd();
  }
}

// ## setup ##
void setup()
{
  // serial: datos
  Serial.begin(57600);

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
}

// # loop ## 
void loop()
{

  leerBluetooth();

  switch (modo)
  {
  case TEST:
    // Serial.println("TEST");
    break;
  case NORMAL:
    // Serial.println("NORMAL");
    break;
  case MANTENIMIENTO:
    // Serial.println("MANTENIMIENTO");
    break;
  }

  for (int i = 0; i < 12; i++) {
    handleKey(i);
  }

  char notasAEnviar[1024];
  strcpy(notasAEnviar,"");

  for (int i = 0; i < 12; i++) {
    if (touched[i]) {
      strcat(notasAEnviar,notes[i]);
    }
  }
  Serial.println(notasAEnviar);

  delay(400);
}
