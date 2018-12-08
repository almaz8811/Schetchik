#include <EEPROM.h>

#define pin 2

boolean statePin, statePinOld;      // Состояние пина
unsigned long lastImpuls = 0;       // Время прошлого импульса
unsigned long dlinaImpuls = 0;      // Длина импульса
unsigned long lastSaveData = 0;     // Предидущее время сохранения данных
float impulsCoint = 0;              // Количество импульсов
float blincsPerHour;                // Сколько таких импульсов поместится в часе
float wat = 0;                      // Текущее количество Ватт
float wattage = 0;                  // Текущее потребление Ватт

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin, INPUT_PULLUP);
  EEPROM.get(0, impulsCoint);     // Считать с ячейки 0 количество импульсов
  EEPROM.get(1, wat);             // Считать с ячейки 1 количество использованных Ватт
}

void loop() {
  // put your main code here, to run repeatedly:
  impuls();
  saveData();
}

void impuls() {    
  statePin = digitalRead(pin);
  
  if (statePin != statePinOld && millis() - lastImpuls > 50) {    // Если состояние пина изменилось (с антидребезгом)
    if (statePin == HIGH) {                                       // И если индикатор горит
      impulsCoint = impulsCoint + 1;                              // Увеличить счетчик импульсов
      dlinaImpuls = millis() - lastImpuls;                        // Вычислить длину импульса
      lastImpuls = millis();
      blincsPerHour = 3600000000 / dlinaImpuls;                   // Вычислить сколько импульсов поместится в часе
      wattage = blincsPerHour / 3200;                             // Текущее потребление Ватт
      wat = (impulsCoint * 1000) / 3200;                          // Сколько Ватт использованно
      Serial.print(String(impulsCoint) + "   ");
      Serial.println(wat);                                        
      Serial.println(wattage);
    }
  }
  
  statePinOld = statePin;
}

void saveData() {
  if (millis() - lastSaveData > 600000) {   // Если прошло 5 минут
    EEPROM.put(0, impulsCoint);             // Сохранить в ячейку 0 количество импульсов
    EEPROM.put(1, wat);                     // Сохранить в ячейку 1 количество использованных Ватт
    Serial.println("Данные сохранены: импульсов - " + String(impulsCoint) + " Ватт - " + String(wat));
    lastSaveData = millis();
  }
}
