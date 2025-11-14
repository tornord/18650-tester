#define S_IDLE 0                                // Idle
#define S_INITIAL_MEASURE_INTERNAL_RESISTANCE 1 // Measuring internal resistance, first time
#define S_INITIAL_CHARGING 2                    // Initial charging
#define S_ERROR 3                               // Error
#define S_CHARGING 4                            // Charging
#define S_DISCHARGING 5                         // Discharging
#define S_WAITING_TO_CHARGE 6                   // Waiting to charge
#define S_WAITING_TO_DISCHARGE 7                // Waiting to discharge
#define S_FINAL_MEASURE_INTERNAL_RESISTANCE 8   // Measuring internal resistance, final time
#define S_FINISHED 9                            // Finished
#define S_NO_INA219 10                          // No INA219
#ifndef BATTERY_H
#define BATTERY_H

#include <WiFiS3.h>
#include <Adafruit_INA219.h>
#include "constants.h"

struct MeasurePoint
{
  float time;
  float voltage;
  float current;
};

class Battery
{
public:
  char name;
  char state;
  float startTime;
  float endTime;
  float internalResistance;
  float voltage;
  float current;
  float inputVoltage;
  float dischargeStartTime;
  float dischargeTotalTime;
  float discharge_mAh;
  float discharge_mWh;
  float chargeStartTime;
  float chargeTotalTime;
  float charge_mAh;
  float charge_mWh;
  float lastLoopTime;
  int chargeOutputPin;
  int dischargeOutputPin;
  Adafruit_INA219 *ina219;
  MeasurePoint measurePoints[MEASURE_POINTS];
  int measurePointsCount;

  Battery(char name, int chargeOutputPin, int dischargeOutputPin, Adafruit_INA219 *ina219);
  void reset();
  void setOutput(int modeD3, int modeD4);
  void measureInternalResistance();
  void printRow(WiFiClient &client, const char *label, float value, int decimals, const char *unit, const char *stringValue);
  const char *stateToString() const;
  void printStatusHtml(WiFiClient &client);
  void printStatusJson(WiFiClient &client);
  void writeMeasurePoint(float time);
  void loop();
};

#endif // BATTERY_H
