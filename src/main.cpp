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
    Serial.println("TEST");
    break;
  case NORMAL:
    Serial.println("NORMAL");
    break;
  case MANTENIMIENTO:
    Serial.println("MANTENIMIENTO");
    break;
  }

  switch (sonido)
  {
  case PIANO:
    Serial.println("PIANO");
    break;
  case BAJO:
    Serial.println("BAJO");
    break;
  }

  delay(100);
}
