#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>
#include <TimeLib.h>

#define BLYNK_PRINT Serial
#define pin 2

char auth[] = "3534391eaa0346bda6090415430ceb23";
char ssid[] = "Keenetic II";
char pass[] = "N5iiwhdU";

WidgetRTC rtc;

boolean statePin, statePinOld;      // Состояние пина
unsigned long lastImpuls = 0;       // Время прошлого импульса
unsigned long dlinaImpuls = 0;      // Длина импульса
unsigned long lastSaveData = 0;     // Предидущее время сохранения данных
float impulsCoint = 0;              // Количество импульсов
float blincsPerHour;                // Сколько таких импульсов поместится в часе
float wat = 0;                      // Текущее количество Ватт
float wattage = 0;                  // Текущее потребление Ватт
float maxWattage = 0;               // Максимальное значение потребления Ватт
float dayWat = 0;                   // Потребление за сутки
float moneyDay = 0;                 // Стоимость за сутки
float moneyMonth = 0;               // стоимость з месяц
int korrect = 0;
int btnKorrect = 0;


BLYNK_CONNECTED() {
  rtc.begin();
}

BLYNK_WRITE(V8)                     // Виджет корректировки
{
  korrect = param.asInt();
}

BLYNK_WRITE(V7)                     // Кнопка корректировки
{
  btnKorrect = param.asInt();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  pinMode(pin, INPUT_PULLUP);
  EEPROM.get(0, impulsCoint);     // Считать с ячейки 0 количество импульсов
  EEPROM.get(1, wat);             // Считать с ячейки 1 количество использованных Ватт
  EEPROM.get(2, maxWattage);      // Считать с ячейки 2 максимальное значение Ватт
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  impuls();
  timerData();
}

void impuls() {
  statePin = digitalRead(pin); 

  if (statePin != statePinOld && millis() - lastImpuls > 50) {    // Если состояние пина изменилось (с антидребезгом)
    if (statePin == HIGH) {                                       // И если индикатор горит
      Blynk.virtualWrite(V5, 255);                                // Зажигаем светодиод
      impulsCoint = impulsCoint + 1;                              // Увеличить счетчик импульсов
      dlinaImpuls = millis() - lastImpuls;                        // Вычислить длину импульса
      lastImpuls = millis();
      blincsPerHour = 3600000000 / dlinaImpuls;                   // Вычислить сколько импульсов поместится в часе
      wattage = blincsPerHour / 3200;                             // Текущее потребление Ватт
      if (maxWattage < wattage) {                                 // Максимальное значение Ватт
        maxWattage = wattage;
      }
      //Serial.print(String(impulsCoint) + "   ");
      Blynk.virtualWrite(V0, wattage);                            // Отправить текущее потребление Ватт      
      Blynk.virtualWrite(V2, maxWattage / 1000);                  // Отправить максимальное значение Ватт
      //Serial.println(wat);
      //Serial.println(wattage);
    }
    Blynk.virtualWrite(V5, 0);                                    // Гасим светодиод
  }

  if (btnKorrect == 1 && korrect > 0) {
    impulsCoint = korrect * 3200;
    wat = (impulsCoint * 1000) / 3200;                          // Сколько Ватт использованно
    Blynk.virtualWrite(V1, wat / 1000);                         // Отправить количество Киловатт
  }

  statePinOld = statePin;
  timeReset();
}

void timerData() {
  if (millis() - lastSaveData > 300000) {   // Если прошло 5 минут
    EEPROM.put(0, impulsCoint);             // Сохранить в ячейку 0 количество импульсов
    EEPROM.put(1, wat);                     // Сохранить в ячейку 1 количество использованных Ватт
    EEPROM.put(2, maxWattage);              // Сохранить в ячейку 2 максимальное значение Ватт
    wat = (impulsCoint * 1000) / 3200;                          // Сколько Ватт использованно
    dayWat = wat / 1000;                                        // Количество Киловатт за сутки
    moneyDay = dayWat * 4.33;                                   // Стоимость за сутки
    moneyMonth = (wat / 1000) * 4.33;                           // Стоимость за месяц
    Blynk.virtualWrite(V1, wat / 1000);                         // Отправить количество Киловатт
    Blynk.virtualWrite(V3, dayWat);                             // Отправить количество Киловатт за сутки
    Blynk.virtualWrite(V4, moneyDay);                           // Отправить стоимость за сутки
    Blynk.virtualWrite(V6, moneyMonth);                         // Отправить стоимость за сутки
    //Serial.println("Данные сохранены: импульсов - " + String(impulsCoint) + " Ватт - " + String(wat));
    lastSaveData = millis();
  }
}

void timeReset() {                         // Обнулить показания за сутки и месяц
  if (hour() == 0 && minute() == 0) {
    maxWattage = 0;
    moneyDay = 0;
    dayWat = 0;
  }

  if (day() == 1 && hour() == 0 && minute() == 0) {
    moneyMonth = 0;
  }
}
