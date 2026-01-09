#include <Wire.h>
#include <U8g2lib.h>  // Установите библиотеку U8g2 через Arduino IDE Library Manager

// Для SSD1306: U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// Если у вас SSD1315 (как в предыдущих сообщениях), используйте:
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Определите шрифты отдельно для удобства изменения
#define CALIB_FONT u8g2_font_6x12_t_cyrillic  // Шрифт для "Калибровка..." (измените здесь для другого размера, напр. u8g2_font_7x13_t_cyrillic)
#define CALIB_DONE_FONT u8g2_font_6x12_t_cyrillic  // Шрифт для "Калибровка завершена" (измените здесь для другого размера)

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
  u8g2.setContrast(1);     // Минимальная яркость (контраст 0-255)

  // Калибровка
  u8g2.clearBuffer();
  u8g2.setFont(CALIB_FONT);  // Устанавливаем шрифт для "Калибровка..."
  u8g2.drawUTF8(0, 20, "Калибровка...");  // Используем drawUTF8 для UTF-8
  u8g2.sendBuffer();

  R0 = calculateR0();

  u8g2.clearBuffer();
  u8g2.setFont(CALIB_DONE_FONT);  // Устанавливаем шрифт для "Калибровка завершена"
  u8g2.drawUTF8(0, 20, "Калибровка завершена");
  u8g2.sendBuffer();
  delay(2000);
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  float volt = (sensorValue / 1023.0) * 5.0;
  float rs = ((5.0 * RL) / volt) - RL;
  float ratio = rs / R0;

  // Calculate PPM using formula for LPG (approximation for natural gas)
  float a = 18.446;
  float b = -0.421;
  float ppm = pow((ratio / a), (1.0 / b));

  // Отображение
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_t_cyrillic);  // Маленький шрифт для заголовка
  u8g2.drawUTF8(0, 15, "Конц. природного газа");

  u8g2.setFont(u8g2_font_10x20_t_cyrillic);  // Больший шрифт для значения PPM
  char buf[10];
  dtostrf(ppm, 1, 0, buf);  // Преобразуем float в строку
  u8g2.drawStr(0, 45, buf);  // Значение PPM (drawStr для ASCII, так как цифры)
  u8g2.drawStr(60, 45, " PPM");  // Единицы (調整 позицию по X, если нужно)

  u8g2.sendBuffer();

  delay(200);  // Уменьшено для быстрой реакции
}
