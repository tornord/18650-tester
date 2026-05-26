#ifndef UTILS_H
#define UTILS_H

#include <WiFiS3.h>

float secs();
void printWithLeadingZero(WiFiClient &client, float v);
String twoDigits(int n);
String formatTime(float s);

#endif // UTILS_H
