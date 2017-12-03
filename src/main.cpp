#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Arduino.h>

// ## constantes ##

// lcd
LiquidCrystal_I2C lcd(0x3f, 20, 4);

// ethernet
byte mac[] = {0xFE, 0xAD, 0xFE, 0xAF, 0xFE, 0xED};
char server[] = "192.168.1.100";
IPAddress ip(192, 168, 1, 101);
EthernetClient client;

// modos
enum modo
{
  MODO_TEST,
  MODO_NORMAL,
  MODO_MANTENIMIENTO
} modo;

// sonidos
enum sonido
{
  SONIDO_PIANO,
  SONIDO_GUITARRA,
  SONIDO_ORGANO,
  SONIDO_EDM,
  SONIDO_CUSTOM_1,
  SONIDO_CUSTOM_2
} sonido;

// tests
enum test
{
  TEST_LCD,
  TEST_LEDS,
  TEST_BUZZER,
  TEST_ETHERNET,
  TEST_GUITARRA,
  TEST_ORGANO,
  TEST_EDM,
  TEST_PIANO
} test;

// request http
enum request
{
  GET,
  POST
} request;

// notas
const int keys[12] = {48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26};
const char notes[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B'};
const int threshold = 2;
bool touched[12];
const int ledNotas = 8;

// BUZZER
const int BUZZER = 4;
const int NOTE_BUZZER = 440;
const int DURATION_BUZZER = 400;

// ## funciones ##

// leer pin capacitivo
uint8_t readCapacitivePin(int pinToMeasure)
{
  volatile uint8_t *port;
  volatile uint8_t *ddr;
  volatile uint8_t *pin;
  byte bitmask;

  port = portOutputRegister(digitalPinToPort(pinToMeasure));
  ddr = portModeRegister(digitalPinToPort(pinToMeasure));
  bitmask = digitalPinToBitMask(pinToMeasure);
  pin = portInputRegister(digitalPinToPort(pinToMeasure));

  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr |= bitmask;
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
  *ddr |= bitmask;

  return cycles;
}

// manejar apretado pin
void handleKey(int index)
{
  int cycles = readCapacitivePin(keys[index]);

  if (!touched[index] && cycles >= threshold)
  {
    touched[index] = true;
    Serial.write(notes[index]);
    digitalWrite(ledNotas, HIGH);
  }

  if (touched[index] && cycles < threshold)
  {
    digitalWrite(ledNotas, LOW);
    touched[index] = false;
  }
}

// mandar un request http
void mandarRequestHttp(int tipoRequest, char url[], char parametro[])
{
  char buffer[128];

  if (client.connect(server, 3000))
  {

    switch (tipoRequest)
    {
    case GET:
      sprintf(buffer, "GET %s HTTP/1.1", url);
      break;
    case POST:
      sprintf(buffer, "POST %s HTTP/1.1", url);
      break;
    }

    client.println(buffer);
    sprintf(buffer, "Host: %s", server);
    client.println(buffer);
    client.println("Connection: close");
    if (parametro != NULL)
    {
      client.println("Content-Type: application/x-www-form-urlencoded");
      sprintf(buffer, "Content-Length: %u\r\n", strlen(parametro));
      client.println(buffer);
      client.print(parametro);
    }
    client.println();
  }
  else
  {
    // TODO: mostrar error de conexion
  }

  delay(2000);

  while (client.available())
  {
    char c = client.read();
  }

  client.stop();
}

// cambiar el tipo de sonido
void cambiarTipoSonido(enum sonido s)
{
  char url[16];
  char tipoSonido[32];

  sonido = s;

  strcpy(url, "/mode");

  switch (sonido)
  {
  case SONIDO_PIANO:
    strcpy(tipoSonido, "modo=piano");
    break;
  case SONIDO_GUITARRA:
    strcpy(tipoSonido, "modo=acoustic");
    break;
  case SONIDO_ORGANO:
    strcpy(tipoSonido, "modo=organ");
    break;
  case SONIDO_EDM:
    strcpy(tipoSonido, "modo=edm");
    break;
  case SONIDO_CUSTOM_1:
    strcpy(tipoSonido, "modo=custom_1");
    break;
  case SONIDO_CUSTOM_2:
    strcpy(tipoSonido, "modo=custom_2");
    break;
  }

  mandarRequestHttp(POST, url, tipoSonido);
  if (modo != MODO_TEST)
    tone(BUZZER, NOTE_BUZZER, DURATION_BUZZER);
  imprimirLcd();
}

// cambiar el modo
void cambiarModo(enum modo m)
{
  modo = m;
  tone(BUZZER, NOTE_BUZZER, DURATION_BUZZER);
  imprimirLcd();
}

// imprimir el lcd
void imprimirLcd()
{
  lcd.clear();

  switch (modo)
  {
  case MODO_TEST:
    lcd.print("MODO: TEST");
    break;
  case MODO_NORMAL:
    lcd.print("MODO: NORMAL");
    break;
  case MODO_MANTENIMIENTO:
    lcd.print("MODO: MANT.");
    break;
  }

  lcd.setCursor(0, 1);

  if (modo == MODO_TEST)
  {
    switch (test)
    {
    case TEST_LCD:
      lcd.print("TEST: LCD");
      break;
    case TEST_LEDS:
      lcd.print("TEST: LEDS");
      break;
    case TEST_BUZZER:
      lcd.print("TEST: BUZZER");
      break;
    case TEST_ETHERNET:
      lcd.print("TEST: ETHERNET");
      break;
    case TEST_GUITARRA:
      lcd.print("TEST: GUITARRA");
      break;
    case TEST_ORGANO:
      lcd.print("TEST: ORGANO");
      break;
    case TEST_EDM:
      lcd.print("TEST: EDM");
      break;
    case TEST_PIANO:
      lcd.print("TEST: PIANO");
      break;
    }
  }
  else
  {
    switch (sonido)
    {
    case SONIDO_PIANO:
      lcd.print("SONIDO: PIANO");
      break;
    case SONIDO_GUITARRA:
      lcd.print("SONIDO: GUITARRA");
      break;
    case SONIDO_ORGANO:
      lcd.print("SONIDO: ORGANO");
      break;
    case SONIDO_EDM:
      lcd.print("SONIDO: EDM");
      break;
    case SONIDO_CUSTOM_1:
      lcd.print("SONIDO: CUSTOM_1");
      break;
    case SONIDO_CUSTOM_2:
      lcd.print("SONIDO: CUSTOM_2");
      break;
    }
  }
}

// leer el bluetooth
void leerBluetooth()
{
  if (Serial1.available())
  {
    int lectura = Serial1.read();
    if (lectura == 'T')
    {
      cambiarModo(MODO_TEST);
    }
    else if (lectura == 'N')
    {
      cambiarModo(MODO_NORMAL);
    }
    else if (lectura == 'M')
    {
      cambiarModo(MODO_MANTENIMIENTO);
    }
    else if (lectura == 'P')
    {
      cambiarTipoSonido(SONIDO_PIANO);
    }
    else if (lectura == 'G')
    {
      cambiarTipoSonido(SONIDO_GUITARRA);
    }
    else if (lectura == 'O')
    {
      cambiarTipoSonido(SONIDO_ORGANO);
    }
    else if (lectura == 'E')
    {
      cambiarTipoSonido(SONIDO_EDM);
    }
    else if (lectura == '1' || lectura == '2')
    {
      if (modo == MODO_MANTENIMIENTO)
      {
        if (lectura == '1')
        {
          cambiarTipoSonido(SONIDO_CUSTOM_1);
        }
        else
        {
          cambiarTipoSonido(SONIDO_CUSTOM_2);
        }
      }
      else
      {
        lcd.clear();
        lcd.print("ERROR: NO SE");
        lcd.setCursor(0, 1);
        lcd.print("PERMITE CUSTOM");
        tone(BUZZER, NOTE_BUZZER, DURATION_BUZZER);
        delay(3000);
        imprimirLcd();
      }
    }
  }
}

// notas de prueba
void testNotes()
{
  Serial.write(notes[0]);
  delay(400);

  Serial.write(notes[4]);
  delay(400);

  Serial.write(notes[7]);
  delay(400);
}

// probar leds
void testLED()
{
  test = TEST_LEDS;
  imprimirLcd();

  for (int i = 0; i < 2; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(ledNotas, HIGH);

    delay(2000);

    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(ledNotas, LOW);

    delay(2000);
  }
}

// probar ethernet
void testEthernet()
{
  test = TEST_ETHERNET;
  imprimirLcd();

  char url[16];
  strcpy(url, "/test");
  mandarRequestHttp(POST, url, NULL);
}

// probar los sonidos
void testSonidos()
{
  test = TEST_GUITARRA;
  cambiarTipoSonido(SONIDO_GUITARRA);
  testNotes();

  test = TEST_ORGANO;
  cambiarTipoSonido(SONIDO_ORGANO);
  testNotes();

  test = TEST_EDM;
  cambiarTipoSonido(SONIDO_EDM);
  testNotes();

  test = TEST_PIANO;
  cambiarTipoSonido(SONIDO_PIANO);
  testNotes();
}

// probar el buzzer
void testBuzzer()
{
  test = TEST_BUZZER;
  imprimirLcd();

  tone(BUZZER, 261, 400);
  delay(400);
  tone(BUZZER, 329, 400);
  delay(400);
  tone(BUZZER, 391, 400);
  delay(400);
  tone(BUZZER, 523, 400);
  delay(400);
}

// probar el lcd
void testLCD()
{
  int i = 0;
  int j = 0;

  test = TEST_LCD;
  imprimirLcd();

  delay(2000);

  lcd.clear();
  lcd.blink();
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < 16; j++)
    {
      lcd.setCursor(j, i);
      delay(800);
    }
  }
  lcd.blink_off();
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

  // init lcd
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Bienvenido!");

  Ethernet.begin(mac, ip);

  delay(2000);

  // init modo
  modo = MODO_NORMAL;

  // init sonido
  cambiarTipoSonido(SONIDO_PIANO);
}

// # loop ##
void loop()
{

  leerBluetooth();

  switch (modo)
  {
  case MODO_TEST:
    testLCD();
    testBuzzer();
    testLED();
    testEthernet();
    testSonidos();
    cambiarModo(MODO_NORMAL);
    break;
  default:
    for (int i = 0; i < 12; i++)
    {
      handleKey(i);
    }
    break;
  }

  delay(5);
}
