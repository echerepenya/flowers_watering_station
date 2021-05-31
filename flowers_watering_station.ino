// Автоматический полив растений с отправкой данных в ThingsSpeak по WiFi и Telegram ботом

// используемые библиотеки
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <RtcDS3231.h>
#include <ThingSpeak.h>
#include <UniversalTelegramBot.h>
// #include <LiquidCrystal_I2C.h>

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

#include "config.h"

// PIN SETUP
static const int SENSOR_PIN = A0;                           // аналоговый пин, куда подключен датчик влажности почвы
static const int SENSOR_POWER_PIN = D7;                     
static const int PUMP_PIN = D6;                             // цифровой пин, с которого включается питание насоса
static const int WATER_LEVEL_PIN = D5;                      // цифровой пин, если на нем возникает питание, то воды в резервуаре нет

// определение констант
const unsigned long measureInterval = 2 * 60 * 60UL;        // Период замеров влажности почвы (каждые два часа). Задается в секундах
const unsigned long wateringInterval = 8 * 60 * 60UL;      // Минимальный перерыв между поливами (12 часов). Задается в секундах

// объявление переменных
//RtcDateTime startTime;
unsigned long lastWatering = 0;
unsigned long lastMeasure = 0;
unsigned long lastThingsSpeak = 0;
unsigned long nextMeasure = 0;
unsigned long lastCheck = 0;

unsigned long botLastTime = 0;                              //last time messages' scan has been done
int botInterval = 1000;                                     // ms beetween new message check
int ledStatus = 0;
float tempCoef = 0.0055;

int minLimitMoistureSensor = 250;                       // нижняя граница измерений датчика влажности
int maxLimitMoistureSensor = 650;                       // верхняя граница измерений датчика влажности
int adrr_min_limit = 0;                                 // адрес хранения минимального уровня в EEPROM
int adrr_max_limit = 5;                                // адрес хранения максимального уровня в EEPROM
int moistureLevel;
int moisturePercent;
int moistureLevelCorrected;
int moisturePercentCorrected;

// инициализация объектов
RtcDS3231<TwoWire> Rtc(Wire);
//LiquidCrystal_I2C lcd(0x3F,16,2);
RtcDateTime now;
RtcTemperature temp;
WiFiClientSecure clientSSL;
UniversalTelegramBot bot(BOTtoken, clientSSL);  

void setup()
{
  Serial.begin(9600); 

  pinMode(SENSOR_POWER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);

  digitalWrite(SENSOR_POWER_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);

  Serial.print("compiled: ");   Serial.print(__DATE__);   Serial.println(__TIME__);   Serial.println();
  Rtc.Begin();
  rtcSetup();

  // если в ЕЕПРОМ нет значений, прописываем значения по умолчанию
  if(EEPROM_int_read(adrr_min_limit) <= 0) EEPROM_int_write(adrr_min_limit, minLimitMoistureSensor);
  if(EEPROM_int_read(adrr_max_limit) <= 0) EEPROM_int_write(adrr_max_limit, maxLimitMoistureSensor);

  minLimitMoistureSensor = EEPROM_int_read(adrr_min_limit);
  maxLimitMoistureSensor = EEPROM_int_read(adrr_max_limit);

  Serial.print("Минимальный лимит: ");
  Serial.println(minLimitMoistureSensor);
  Serial.print("Максимальный лимит: ");
  Serial.println(maxLimitMoistureSensor);

//  lcd.init();
    
  // Set WiFi to station mode and disconnect from an AP if it was Previously connected
  WiFi.mode(WIFI_STA);                              // переводим NodeMCU в режим клиента, а не точки доступа
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting WiFi: ");
  Serial.println(ssid);
//  lcd.backlight();
//  lcd.setCursor(0,0);
//  lcd.print("Connecting WiFi: ");
//  lcd.setCursor(0,1);
//  lcd.print(ssid);
//  lcd.noBacklight();

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("WiFi connected. "); Serial.print("IP address: "); Serial.println(WiFi.localIP()); Serial.println();
  clientSSL.setInsecure();
  sendMessage("Перезагрузка завершена");
//  startTime = Rtc.GetDateTime();
}

void loop()
{
  // проверка работы часов
  if (!Rtc.IsDateTimeValid()) 
  {
    Serial.println("RTC lost confidence in the DateTime!");
  }

  // проверка входящих собщений от Telegram бота
  if (millis() > botLastTime + botInterval)
  {
    checkIncomingMessages();   
    tcpCleanup();
  }
  
  // Если с момента последнего замера влажности прошло времени больше, чем заданный интервал измерений, меряем влажность почвы
  if (Rtc.GetDateTime() > (lastMeasure + measureInterval))
  {
    moistureLevel = getMoisture();
    moisturePercent = getMoisturePercent(moistureLevel);
    lastMeasure = Rtc.GetDateTime();

    Serial.print("Влажность грунта: ");
    Serial.print(moisturePercent);
    Serial.print("%. ");
  
    temp = Rtc.GetTemperature();
    Serial.print("Температура: ");
    Serial.print(temp.AsFloatDegC());
    Serial.println("C");
    Serial.println();
  
    sendtoThingsSpeak(moisturePercent, moistureLevel, temp.AsFloatDegC());
    lastThingsSpeak = millis();

    // Если влажность почвы > 30% -> земля еще влажная, ждем дальше
    if(moisturePercent > 50)
    {
      Serial.print("Почва еще влажная. ");
      nextMeasure = Rtc.GetDateTime() - lastMeasure + measureInterval;    
      Serial.print(nextMeasure/60);
      Serial.println(" мунут до следующего замера");
      Serial.println();
      return;
    }
  
    Serial.print("Почва сухая. ");
  
//    String message = "Почва сухая, пора полить цветы";
    sendMessage("Почва сухая, пора полить цветы");
  
    // проверка времени прошедшего с последнего полива
    if(Rtc.GetDateTime() < (lastWatering + wateringInterval))
    {
      Serial.println("Но с момента последнего полива прошло недостаточно времени");
      nextMeasure = lastMeasure + measureInterval - Rtc.GetDateTime();
      Serial.print(nextMeasure/60);
      Serial.println(" минут до следующего замера");
      Serial.println();
      return;
    }
    Serial.println("Поливаем...");
  
    bool wateringResult = waterIt();
    moisturePercent = getMoisturePercent(getMoisture());
   
    if(!wateringResult)
    {
      Serial.println("Нет воды в резервуаре");
      String message = "Нет воды в резервуаре. Влажность почвы: " + String(moisturePercent) + "%";
      sendMessage(message);
    }
    else
    {
      Serial.print("Полив выполнен. ");
      Serial.print("Новая влажность почвы: ");
      Serial.print(moisturePercent);
      Serial.println("%");
  
      String message = "Полив выполнен. Влажность почвы: " + String(moisturePercent) + "%";
      sendMessage(message);
    }
  
    nextMeasure = lastMeasure + measureInterval - Rtc.GetDateTime();
    Serial.print(nextMeasure/60);
    Serial.println(" минут до следующего замера");
    Serial.println();
  }
}
//    Serial.print("Memory: ");
//    Serial.println(ESP.getFreeHeap());
