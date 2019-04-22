//logb.cpp
#include "logb.h"
SdFat SD;

#if defined(ESP8266)
HTTPClient http;
#endif

DateTime NoTime()
{
  return 1702511185;
}

DateTime UnixTime(int tz)
{
#if defined(ESP8266)
  time_t now = 0;
  int timezone = tz * 3600;
  while (now < 958881900)
  {
    configTime(timezone, 0, "pool.ntp.org", "time.nist.gov", "time.windows.com");
    while (!time(nullptr))
    {
    }
    delay(50);
    now = time(nullptr);
  }
  return now;
#endif
}

void CreateName(DateTime time)
{
  set.ArduinoName = set.device_id + "_" + String(time.unixtime());
}

String wz(int n)
{
  String r = "0";
  if (n < 10)
  {
    r += n;
  }
  else
  {
    r = n;
  }
  return r;
}
String Time(DateTime time)
{
  String date = "";
  set.date = time;
  if (time.unixtime() == 1702511185)
  {
    unsigned long m = set.previousMillis - set.firstMillis;
    date = String(m / 1000) + "." + String((m - m / 1000 * 1000) / 100);
  }
  else
  {
    date += time.year();
    date += "-";
    date += wz(time.month());
    date += "-";
    date += wz(time.day());
    date += " ";
    date += wz(time.hour());
    date += ":";
    date += wz(time.minute());
    date += ":";
    date += wz(time.second());
  }
  return date;
}

void dateTime(uint16_t *date, uint16_t *time)
{
  DateTime now = set.date;
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void Send()
{
  if (set.toComma)
  {
    set.fulldata.replace('.', ',');
  }
  for (int j = 0; j < set.where.length(); j++)
  {
    char w = set.where.charAt(j);

    if (w == 'a')
    {
      Serial.println(set.fulldata);
    }

    if (w == 'b')
    {
      SdFile::dateTimeCallback(dateTime);
      if (set.DB == false)
      {
        SD.begin(SS);
      }
      String file = set.ArduinoName + ".txt";
      File Sd = SD.open(file, FILE_WRITE);
      if (Sd)
      {
        Sd.println(set.fulldata);
      }
      Sd.close();
    }

    if (w == 'c')
    {
#if defined(ESP8266)
      http.begin("http://api.logb.hu/v1.1/upload.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      set.cloud += "oszlop=" + String(set.sensor_count) + "&ma=" + set.ArduinoName + "&pin=" + set.pin + "&device=" + set.device_id + "&time=" + Time(set.date);
      int httpCode = http.POST(set.cloud);
      http.end();
      //Serial.println(httpCode);
#endif
    }
  }
  set.DB = true;
  set.sensor_count = 0;
  set.cloud = "";
  set.fulldata = "";
}

void AddData(String id, String header, String data)
{
  if (set.DB == false)
  {
    if (set.date.unixtime() == 1702511185)
    {
      set.firstMillis = set.previousMillis + set.timeInterval;
    }
    set.sensor_count++;
    set.fulldata += header + set.seperate;
    set.cloud += "logb" + String(set.sensor_count) + "=" + header + "&sensor" + String(set.sensor_count) + "=" + id + "&";
  }
  else
  {
    set.sensor_count++;
    set.fulldata += data;
    set.fulldata += set.seperate;
    set.cloud += "logb" + String(set.sensor_count) + "=" + data + "&";
  }
}