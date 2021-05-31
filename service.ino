void rtcSetup()
{
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.print("Скомпилировано: ");
  Serial.print(printDateTime(compiled));
  Serial.println();

  if (!Rtc.IsDateTimeValid()) 
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
  
    Serial.println("RTC lost confidence in the DateTime!");
  
    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue
  
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  now = Rtc.GetDateTime();
  if (now < compiled) 
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
String printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return datestring;
}

void tcpCleanup()
{
  while(tcp_tw_pcbs!=NULL)
  {
    tcp_abort(tcp_tw_pcbs);
    Serial.println();
    Serial.println("Memory Cleaned");
    Serial.println();
  }
}
