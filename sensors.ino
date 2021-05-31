int getMoisture()
{
  int i = 1;
  int sensorValue = 0;
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(2000);
  
  while (i <= 10)
  {
    int singleMeasurement = analogRead(SENSOR_PIN);
    sensorValue += singleMeasurement;
    delay(500);
    i++;
  }
  
  digitalWrite(SENSOR_POWER_PIN, LOW);
  sensorValue = sensorValue / 10;
  return sensorValue;
}

int getMoisturePercent(int value)
{
  value = constrain(value, minLimitMoistureSensor, maxLimitMoistureSensor);
  int moisturePercent = map(value, maxLimitMoistureSensor, minLimitMoistureSensor, 0, 100);
  return moisturePercent;
}
