#include "Battery.h"
#include <Arduino.h>
#include <math.h>
#include "constants.h"
#include "utils.h"

Battery::Battery(char name, int chargeOutputPin, int dischargeOutputPin, Adafruit_INA219 *ina219)
{
  this->name = name;
  this->chargeOutputPin = chargeOutputPin;
  this->dischargeOutputPin = dischargeOutputPin;
  this->ina219 = ina219;
  lastLoopTime = 0.0;
  reset();
  pinMode(chargeOutputPin, OUTPUT);
  pinMode(dischargeOutputPin, OUTPUT);
}

void Battery::reset()
{
  state = S_IDLE;
  startTime = secs();
  voltage = 0.0;
  current = 0.0;
  inputVoltage = 0.0;
  dischargeStartTime = 0;
  dischargeTotalTime = 0;
  chargeStartTime = 0;
  chargeTotalTime = 0;
  endTime = 0;
  internalResistance = -1.0;
  charge_mAh = 0.0;
  charge_mWh = 0.0;
  discharge_mAh = 0.0;
  discharge_mWh = 0.0;
  measurePointsCount = 0;
}

void Battery::setOutput(int modeD3, int modeD4)
{
  digitalWrite(chargeOutputPin, modeD3);
  digitalWrite(dischargeOutputPin, modeD4);
}

void Battery::measureInternalResistance()
{
  setOutput(1, 0);
  if (state == S_NO_INA219)
  {
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  float v0 = ina219->getBusVoltage_V();
  Serial.print("V0=");
  Serial.println(v0, 3);
  if (v0 <= ERROR_VOLTAGE)
  {
    internalResistance = -98.9;
  }
  setOutput(1, 1);
  delay(2000);
  float v1 = ina219->getBusVoltage_V();
  Serial.print("V1=");
  Serial.println(v1, 3);
  float i = ina219->getCurrent_mA() / 1000.0;
  Serial.print("I=");
  Serial.println(i, 4);
  setOutput(1, 0);
  delay(10);
  if (i == 0.0)
  {
    internalResistance = -99.9;
    return;
  }
  internalResistance = (v1 - v0) / i;
}

void Battery::printRow(WiFiClient &client, const char *label, float value, int decimals, const char *unit, const char *stringValue)
{
  client.println("<div class=\"row\">");
  client.print("<span>");
  client.print(label);
  client.println("</span>");
  client.print("<span>");
  if (decimals < 0)
  {
    client.print(stringValue);
  }
  else
  {
    client.print(value, decimals);
    if (strlen(unit) > 0)
    {
      client.print(" ");
      client.print(unit);
    }
  }
  client.println("</span>");
  client.println("</div>");
}

const char *Battery::stateToString() const
{
  switch (state)
  {
  case S_IDLE:
    return "IDLE";
  case S_INITIAL_MEASURE_INTERNAL_RESISTANCE:
    return "INRE0";
  case S_INITIAL_CHARGING:
    return "ICHG";
  case S_ERROR:
    return "ERR";
  case S_CHARGING:
    return "CHG";
  case S_DISCHARGING:
    return "DCHG";
  case S_WAITING_TO_CHARGE:
    return "WCHG";
  case S_WAITING_TO_DISCHARGE:
    return "WDCHG";
  case S_FINISHED:
    return "FIN";
  case S_NO_INA219:
    return "NOINA";
  default:
    return "UKWN";
  }
}

void Battery::printStatusHtml(WiFiClient &client)
{
  float t1 = endTime;
  if (t1 == 0.0)
    t1 = secs();
  float elapsedSecs = t1 - startTime;
  const char *stateStr = stateToString();
  printRow(client, "Battery", 0, -1, "", String(name).c_str());
  printRow(client, "State", 0, -1, "", stateStr);
  String t = formatTime(elapsedSecs);
  printRow(client, "Elapsed time", 0, -1, "", t.c_str());
  printRow(client, "Voltage", voltage, 3, "V", "");
  printRow(client, "Current", current, 1, "mA", "");
  printRow(client, "Input voltage", inputVoltage, 3, "V", "");
  float power = abs(voltage * current) / 1000.0;
  printRow(client, "Power", power, 2, "W", "");
  if (internalResistance > 0.0)
  {
    printRow(client, "Int. resist.", 100.0 * internalResistance, 0, "cΩ", "");
  }
  if (state == S_WAITING_TO_DISCHARGE)
  {
    printRow(client, "Waiting to DCHG", dischargeStartTime - secs(), 1, "s", "");
  }
  if (dischargeTotalTime != 0.0)
  {
    String td = formatTime(dischargeTotalTime);
    printRow(client, "Dischg time", 0, -1, "", td.c_str());
    printRow(client, "Dischg capacity.", discharge_mAh, 1, "mAh", "");
    printRow(client, "Dischg energy", discharge_mWh, 1, "mWh", "");
  }
  if (state == S_WAITING_TO_CHARGE)
  {
    printRow(client, "Waiting to CHG", chargeStartTime - secs(), 1, "s", "");
  }
  if (chargeTotalTime != 0.0)
  {
    String tc = formatTime(chargeTotalTime);
    printRow(client, "Charge time", 0, -1, "", tc.c_str());
    printRow(client, "Charge capacity.", charge_mAh, 1, "mAh", "");
    printRow(client, "Charge energy", charge_mWh, 1, "mWh", "");
  }
}

void Battery::printStatusJson(WiFiClient &client)
{
  float t1 = endTime;
  if (t1 == 0.0)
    t1 = secs();
  float elapsedSecs = t1 - startTime;
  const char *stateStr = stateToString();
  client.print("{");
  client.print("\"name\":\"");
  client.print(name);
  client.print("\",");
  client.print("\"state\":\"");
  client.print(stateStr);
  client.print("\",");
  client.print("\"elapsedTime\":");
  client.print(elapsedSecs);
  client.print(",");
  client.print("\"voltage\":");
  client.print(voltage, 3);
  client.print(",");
  client.print("\"current\":");
  client.print(current / 1000.0, 4);
  client.print(",");
  client.print("\"inputVoltage\":");
  client.print(inputVoltage, 3);
  client.print(",");
  client.print("\"power\":");
  float power = abs(voltage * current) / 1000.0;
  client.print(power, 2);
  client.print(",");
  client.print("\"internalResistance\":");
  client.print(internalResistance, 4);
  if (state == S_WAITING_TO_DISCHARGE)
  {
    client.print(",");
    client.print("\"waitingToDischargeTime\":");
    client.print(dischargeStartTime - secs());
  }
  if (dischargeTotalTime != 0.0)
  {
    client.print(",");
    client.print("\"dischargeTotalTime\":");
    client.print(dischargeTotalTime);
    client.print(",");
    client.print("\"dischargeCapacity\":");
    client.print(discharge_mAh, 1);
    client.print(",");
    client.print("\"dischargeEnergy\":");
    client.print(discharge_mWh, 1);
  }
  if (state == S_WAITING_TO_CHARGE)
  {
    client.print(",");
    client.print("\"waitingToChargeTime\":");
    client.print(chargeStartTime - secs());
  }
  if (chargeTotalTime != 0.0)
  {
    client.print(",");
    client.print("\"chargeTotalTime\":");
    client.print(chargeTotalTime);
    client.print(",");
    client.print("\"chargeCapacity\":");
    client.print(charge_mAh);
    client.print(",");
    client.print("\"chargeEnergy\":");
    client.print(charge_mWh, 1);
  }

  client.print(",\"measurePoints\":[");
  for (int i = 0; i < measurePointsCount; i++)
  {
    MeasurePoint &mp = measurePoints[i];
    if (i > 0)
    {
      client.print(",");
    }
    client.print("{\"time\":");
    client.print(mp.time, 1);
    client.print(",\"voltage\":");
    client.print(mp.voltage, 3);
    client.print(",\"current\":");
    client.print(mp.current / 1000.0, 4);
    client.print("}");
  }
  client.print("]");
  client.println("}");
}

void Battery::writeMeasurePoint(float time)
{
  int i = 0;
  if (measurePointsCount > 0)
  {
    i = floor(time / TIME_BETWEEN_MEASURE_POINTS) + 1;
  }
  if (i >= MEASURE_POINTS)
  {
    return;
  }
  if (i >= measurePointsCount)
  {
    measurePointsCount = i + 1;
  }
  MeasurePoint &mp = measurePoints[i];
  mp.time = time;
  mp.voltage = voltage;
  mp.current = current;
}

void Battery::loop()
{
  float loopDeltaSecs = 0;
  if (lastLoopTime > 0)
  {
    loopDeltaSecs = secs() - lastLoopTime;
  }
  if (loopDeltaSecs > 0.0 && loopDeltaSecs < 2.0)
  {
    return;
  }
  float loopDeltaHours = loopDeltaSecs / 3600.0;
  lastLoopTime = secs();
  Serial.print(name);
  if (state == S_NO_INA219)
  {
    voltage = 0.0;
    current = 0.0;
    inputVoltage = 0.0;
    Serial.println(" no INA219");
    return;
  }
  voltage = ina219->getBusVoltage_V();
  current = ina219->getCurrent_mA();
  float shuntVoltage = ina219->getShuntVoltage_mV() / 1000.0;
  inputVoltage = voltage + shuntVoltage;
  Serial.print(" ");
  const char *stateStr = stateToString();
  Serial.println(stateStr);
  Serial.print(">U");
  Serial.print(name);
  Serial.print(":");
  Serial.println(voltage, 3);
  Serial.print(">I");
  Serial.print(name);
  Serial.print(":");
  Serial.println(current, 1);
  if (voltage <= NO_BATTERY_VOLTAGE)
  {
    if (state != S_IDLE)
    {
      reset();
      state = S_IDLE;
    }
    return;
  }
  if (state == S_ERROR)
  {
    setOutput(1, 0);
    if (voltage > ERROR_VOLTAGE && internalResistance < ERROR_INTERNAL_RESISTANCE)
    {
      reset();
      state = S_IDLE;
    }
    return;
  }
  if (state == S_IDLE)
  {
    if (internalResistance == -1 && voltage > ERROR_VOLTAGE)
    {
      measureInternalResistance();
      if (internalResistance > 0.0)
      {
        state = S_INITIAL_MEASURE_INTERNAL_RESISTANCE;
      }
    }
    return;
  }
  if (state == S_INITIAL_MEASURE_INTERNAL_RESISTANCE)
  {
    if (internalResistance >= ERROR_INTERNAL_RESISTANCE)
    {
      state = S_ERROR;
      setOutput(1, 0);
      return;
    }
    state = S_INITIAL_CHARGING;
    setOutput(0, 0);
    return;
  }
  if (state == S_INITIAL_CHARGING)
  {
    if (voltage > FULLY_CHARGED_VOLTAGE && (secs() - startTime) > 30 && current < MIN_CHARGING_CURRENT)
    {
      state = S_WAITING_TO_DISCHARGE;
      setOutput(1, 0);
      dischargeStartTime = secs() + DISCHARGE_WAIT_TIME;
    }
    return;
  }
  if (state == S_WAITING_TO_DISCHARGE)
  {
    if (secs() > dischargeStartTime)
    {
      state = S_DISCHARGING;
      discharge_mAh = 0.0;
      discharge_mWh = 0.0;
      setOutput(1, 1);
    }
    return;
  }
  if (state == S_DISCHARGING)
  {
    dischargeTotalTime = secs() - dischargeStartTime;
    writeMeasurePoint(dischargeTotalTime);
    if (current < 0.0)
    {
      discharge_mAh -= current * loopDeltaHours;
      discharge_mWh -= current * voltage * loopDeltaHours;
    }
    if (voltage < DISCHARGE_CUTOFF_VOLTAGE)
    {
      state = S_WAITING_TO_CHARGE;
      chargeStartTime = secs() + CHARGE_WAIT_TIME;
      setOutput(1, 0);
    }
    return;
  }
  if (state == S_WAITING_TO_CHARGE)
  {
    if (secs() > chargeStartTime)
    {
      state = S_CHARGING;
      charge_mAh = 0.0;
      charge_mWh = 0.0;
      setOutput(0, 0);
    }
    return;
  }
  if (state == S_CHARGING)
  {
    chargeTotalTime = secs() - chargeStartTime;
    if (current > 0.0)
    {
      charge_mAh += current * loopDeltaHours;
      charge_mWh += current * voltage * loopDeltaHours;
    }
    if (voltage > FULLY_CHARGED_VOLTAGE && (secs() - chargeStartTime) > 30 && current < MIN_CHARGING_CURRENT)
    {
      state = S_FINAL_MEASURE_INTERNAL_RESISTANCE;
      endTime = secs();
      setOutput(1, 0);
    }
    return;
  }
  if (state == S_FINAL_MEASURE_INTERNAL_RESISTANCE)
  {
    measureInternalResistance();
    if (internalResistance > 0.0)
    {
      state = S_FINISHED;
    }
    else
    {
      state = S_ERROR;
    }
    return;
  }
  if (state == S_FINISHED)
  {
    setOutput(1, 0);
    if (voltage < NO_BATTERY_VOLTAGE)
    {
      reset();
      state = S_IDLE;
    }
    return;
  }
}
