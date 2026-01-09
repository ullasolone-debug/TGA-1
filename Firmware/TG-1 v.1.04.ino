#include <Wire.h>
#include <U8g2lib.h>  // Установите библиотеку U8g2 через Arduino IDE Library Manager

// Для SSD1315 используйте SH1106, так как они совместимы
// U8G2_CH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define SENSOR_PIN A0
#define RL 20.0  // Load resistor in kOhms
#define RO_CLEAN_AIR_FACTOR 9.9  // Rs/Ro in clean air for MQ-6

float R0 = 0.0;

float calculateR0() {
  int samples = 50;
  float sum = 0.0;
  for (int i = 0; i < samples; i++) {
    int sensorValue = analogRead(SENSOR_PIN);
    float volt = (sensorValue / 1023.0) * 5.0;
    float rs = ((5.0 * RL) / volt) - RL;
    sum += rs;
    delay(50);
  }
  float rs_avg = sum / samples;
  return rs_avg / RO_CLEAN_AIR_FACTOR;
}

void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print();  // Включаем поддержку UTF-8 для русского текста
  u8g2.setContrast(15);     // Регулировка яркости (контраст 0-255)

  // Калибровка
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13_t_cyrillic);  // Маленький шрифт с поддержкой кириллицы
  u8g2.drawUTF8(10, 25, "ТГ-1 КАНАРЕЙКА");  // Используем drawUTF8 для UTF-8
  u8g2.drawUTF8(40, 45, "v.1.04");
  u8g2.sendBuffer();

  R0 = calculateR0();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_8x13_t_cyrillic);
  u8g2.drawUTF8(25, 25, "Калибровка ");
  u8g2.drawUTF8(28, 45, "завершена");
  u8g2.sendBuffer();
  delay(2000);
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  float volt = (sensorValue / 1023.0) * 5.0;
  float rs = ((5.0 * RL) / volt) - RL;
  float ratio = rs / R0;

  // Calculate PPM using formula for LPG (approximation for natural gas)
  float a = 15.00;
  float b = -0.35;
  float ppm = pow((ratio / a), (1.0 / b));

  // Отображение
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13_t_cyrillic);  // Маленький шрифт для заголовка
  u8g2.drawUTF8(5, 15, "Концентрация газа");

  u8g2.setFont(u8g2_font_10x20_t_cyrillic);  // Больший шрифт для значения PPM
  char buf[10];
  dtostrf(ppm, 1, 0, buf);  // Преобразуем float в строку
  u8g2.drawStr(10, 55, buf);  // Значение PPM (drawStr для ASCII, так как цифры)
  u8g2.drawStr(60, 55, " PPM");  // Единицы (корректируй позицию по X, если нужно)

  u8g2.sendBuffer();

  delay(300);  // Уменьшено для быстрой реакции
}
