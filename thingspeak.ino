void sendtoThingsSpeak(int moisturePercent, int moistureLevel, int temperature)
{
  if(moistureLevel > 20 && temperature > 0)
  {
    WiFiClient client;
    ThingSpeak.begin(client);
    ThingSpeak.setField(1, moisturePercent);
    ThingSpeak.setField(2, moistureLevel);
    ThingSpeak.setField(3, temperature);
    ThingSpeak.writeFields(channelID, writeAPIKey);
    Serial.println("Данные оправлены на сервер");
    Serial.println("");
  }
}
