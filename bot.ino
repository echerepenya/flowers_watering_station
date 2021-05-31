void checkIncomingMessages()
{
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//    Serial.print("bot.getUpdates. Memory: ");
//    Serial.print(ESP.getFreeHeap());
//    Serial.println();
     
    while(numNewMessages)
    {
//      Serial.print("Получена команда. Memory: ");
//      Serial.print(ESP.getFreeHeap());
//      Serial.println();
      
      Serial.print("Обработка сообщений: ");
      Serial.print(String(numNewMessages));
      Serial.println();
      handleNewMessages(numNewMessages, bot);    
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    botLastTime = millis();
}

void handleNewMessages(int numNewMessages, UniversalTelegramBot &bot)
{
  for (int i=0; i<numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/minlim-") {
      EEPROM_int_write(adrr_min_limit, minLimitMoistureSensor - 50);
      minLimitMoistureSensor = EEPROM_int_read(adrr_min_limit);
      bot.sendMessage(chat_id, String(minLimitMoistureSensor), "");
    }

    if (text == "/minlim+") {
      EEPROM_int_write(adrr_min_limit, minLimitMoistureSensor + 50);
      minLimitMoistureSensor = EEPROM_int_read(adrr_min_limit);
      bot.sendMessage(chat_id, String(minLimitMoistureSensor), "");
    }

    if (text == "/maxlim-") {
      EEPROM_int_write(adrr_max_limit, maxLimitMoistureSensor - 50);
      maxLimitMoistureSensor = EEPROM_int_read(adrr_max_limit);
      bot.sendMessage(chat_id, String(maxLimitMoistureSensor), "");
    }

    if (text == "/maxlim+") {
      EEPROM_int_write(adrr_max_limit, maxLimitMoistureSensor + 50);
      maxLimitMoistureSensor = EEPROM_int_read(adrr_max_limit);
      bot.sendMessage(chat_id, String(maxLimitMoistureSensor), "");
    }
    
    if (text == "/waterOn") {
      digitalWrite(PUMP_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      bot.sendMessage(chat_id, "Полив включен", "");
    }

    if (text == "/waterOff") {
      ledStatus = 0;
      digitalWrite(PUMP_PIN, LOW);    // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Полив выключен", "");
    }

    if (text == "/status") {
      String statusMessage = "";
      if(ledStatus){
        statusMessage += "Светодиод включен\n";
      } else {
        statusMessage += "Светодиод выключен\n";
      }

      minLimitMoistureSensor = EEPROM_int_read(adrr_min_limit);
      maxLimitMoistureSensor = EEPROM_int_read(adrr_max_limit);
      
      statusMessage += "Минимальный порог влажности почвы ";
      statusMessage += String(minLimitMoistureSensor);
      statusMessage += ", максимальный ";
      statusMessage += String(maxLimitMoistureSensor);
      statusMessage += ".\n";
      
      statusMessage += "Датчик влажности: ";
      int AnalogRead = getMoisture();
      statusMessage += String(AnalogRead);
      statusMessage += "\n";

      statusMessage += "Влажность почвы: ";
      statusMessage += String(getMoisturePercent(AnalogRead));
      statusMessage += "%\n";

      temp = Rtc.GetTemperature();
      now = Rtc.GetDateTime();
      
      statusMessage += "Температура: ";
      statusMessage += String(temp.AsFloatDegC());
      statusMessage += "C\n";
      statusMessage += "Время: ";
      statusMessage += String(printDateTime(now));
      statusMessage += "\n";

      statusMessage += "Предыдущий замер ";
      statusMessage += String((now - lastMeasure)/60);
      statusMessage += " минут назад, ";
      
      nextMeasure = lastMeasure + measureInterval - now;
      statusMessage += String(nextMeasure/60);
      statusMessage += " минут до следующего замера";
      statusMessage += "\n";

      statusMessage += "Memory: ";
      statusMessage += String(ESP.getFreeHeap());
      
      bot.sendMessage(chat_id, statusMessage, "");
    }

    if (text == "/start")
    {
      String welcome = "Привет, " + from_name + ". Я твой домашний Bot. Моя работа, следить за тем, чтобы твои цветы не засохли.\n\n";
      welcome += "/status : показать состояние дел\n";
      welcome += "/waterOn : включить полив\n";
      welcome += "/waterOff : выключить полив\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void sendMessage(String msg)
{
  Serial.print("Отправляем сообщение в Телеграм... ");
//  WiFiClientSecure clientSSL;
//  UniversalTelegramBot bot(BOTtoken, clientSSL);
  bot.sendMessage(chatID_Evgeniy, msg, "");
  Serial.println("сообщение отправлено.");
}
