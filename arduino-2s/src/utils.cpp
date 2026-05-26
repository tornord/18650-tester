#include <Arduino.h>
#include <WiFiS3.h>

float secs()
{
  return (float)(millis() / 1000.0);
}

void printWithLeadingZero(WiFiClient &client, float v)
{
  v = round(v);
  if (v < 10)
  {
    client.print("0");
  }
  client.print(v, 0);
}

String twoDigits(int n)
{
  if (n < 10)
    return "0" + String(n);
  return String(n);
}

String formatTime(float s)
{
  s = round(s);
  int m = floor(s / 60.0);
  int h = floor(m / 60.0);
  m -= h * 60;
  int sec = s - (h * 3600 + m * 60);

  String result = twoDigits(h) + ":" + twoDigits(m) + ":" + twoDigits(sec);
  return result;
}
