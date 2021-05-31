bool waterIt()
{
  int wateringCount = 0;
  moistureLevel = getMoisture();
  moisturePercent = getMoisturePercent(moistureLevel);

  temp = Rtc.GetTemperature();
    
  while(wateringCount < 10 && moisturePercent < 80)
  {
    bool isTankEmpty = digitalRead(WATER_LEVEL_PIN) == HIGH;
    if(isTankEmpty)
    {
      return false;
    }
    digitalWrite(PUMP_PIN, HIGH);
    Serial.print("Насос включен (");
    Serial.print(wateringCount+1);
    Serial.print("), ");
    delay(2*60*1000L);
    digitalWrite(PUMP_PIN, LOW);
    Serial.println("насос выключен.");
    wateringCount++;
    lastWatering = Rtc.GetDateTime();

    Serial.print("Ждем две минуты... ");
    delay(2*60*1000L);
    Serial.println("меряем влажность.");
    moistureLevel = getMoisture();
    moisturePercent = getMoisturePercent(moistureLevel);
    Serial.print("Влажность: ");
    Serial.println(moisturePercent);

    sendtoThingsSpeak(moisturePercent, moistureLevel, temp.AsFloatDegC());

    String wMessage = "Полив ";
    wMessage += String(wateringCount);
    wMessage += " выполнен. Влажность: ";
    wMessage += String(moisturePercent);
    wMessage += "%\n";
    sendMessage(wMessage);
  }
  return true;
}
