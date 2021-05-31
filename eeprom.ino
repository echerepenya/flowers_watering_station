// чтение EEPROM
int EEPROM_int_read(int addr) {
  EEPROM.begin(512);
  delay(10);
  byte raw[4];
  for (byte i = 0; i < 4; i++) raw[i] = EEPROM.read(addr + i);
  int &num = (int&)raw;
  delay(10);
  EEPROM.end();
  return num;
}

// запись EEPROM
void EEPROM_int_write(int addr, int num) {
  if (EEPROM_int_read(addr) != num)  //если сохраняемое отличается, то записываем
  {
    Serial.print("Пишу в ЕПРОМ... ");
    EEPROM.begin(512);
    delay(10);
    byte raw[4];
    (int&)raw = num;
    for (byte i = 0; i < 4; i++) EEPROM.write(addr + i, raw[i]);
    EEPROM.commit();
    delay(10);
    EEPROM.end();
    Serial.println("перезаписали");
  }
  else
  {
    Serial.println("В ЕЕПРОМ хранятся те же значения, перезаписывать нет необходимости");
  }
  
}
