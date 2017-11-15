#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

LiquidCrystal_I2C lcd(0x3f, 20, 4);

enum modo
{
  TEST,
  NORMAL,
  MANTENIMIENTO
} modo;

enum sonido
{
  PIANO,
  BAJO
} sonido;

 
const int keys[8] = { 22, 24, 26, 28, 30, 32, 34, 36 };
const int threshold[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };

bool touched[8];

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

void handleKey(int index) {
  int cycles = readCapacitivePin(keys[index]);

  if (!touched[index] && cycles >= threshold[index]) {
    touched[index] = true;
    Serial.print("Index: ");
    Serial.println(index);
  }

  if (touched[index] && cycles < threshold[index]) {
    touched[index] = false;
  }
}

void setup()
{
  // serial: datos
  Serial.begin(9600);

  // serial1: bluetooth
  Serial1.begin(9600);

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

void cambiarTipoSonido()
{
  return;
}

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
      sonido = PIANO;
      cambiarTipoSonido();
    }
    else if (lectura == 'B')
    {
      sonido = BAJO;
      cambiarTipoSonido();
    }

    imprimirLcd();
  }
}

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

  switch (sonido)
  {
  case PIANO:
    // Serial.println("PIANO");
    break;
  case BAJO:
    // Serial.println("BAJO");
    break;
  }

  for (int i = 0; i < 8; i++) {
    handleKey(i);
  }

  delay(50);
}
