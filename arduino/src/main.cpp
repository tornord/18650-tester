#include "WiFiS3.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;          // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

float DISCHARGE_WAIT_TIME = 1.0 * 3600; // seconds, 12*3600 = 6 hours
float CHARGE_WAIT_TIME = 60;            // seconds, 60
float DISCHARGE_CUTOFF_VOLTAGE = 3.00;  // volts, 3.00
float ERROR_VOLTAGE = 2.50;             // volts
float ERROR_INTERNAL_RESISTANCE = 0.30; // ohms
float NO_BATTERY_VOLTAGE = 0.10;        // volts
float FULLY_CHARGED_VOLTAGE = 4.05;     // volts
float MIN_CHARGING_CURRENT = 70;        // mA

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

  Battery(char name, int chargeOutputPin, int dischargeOutputPin, Adafruit_INA219 *ina219)
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

  void reset()
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
  };

  void setOutput(int modeD3, int modeD4)
  {
    digitalWrite(chargeOutputPin, modeD3);
    digitalWrite(dischargeOutputPin, modeD4);
  };

  void measureInternalResistance()
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
  };

  void printRow(WiFiClient &client, const char *label, float value, int decimals, const char *unit, const char *stringValue)
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

  const char *stateToString() const
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

  void printStatusHtml(WiFiClient &client)
  {
    float t1 = endTime;
    if (t1 == 0.0)
    {
      t1 = secs();
    }
    float elapsedSecs = t1 - startTime;

    // convert state to string
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

  void printStatusJson(WiFiClient &client)
  {
    float t1 = endTime;
    if (t1 == 0.0)
    {
      t1 = secs();
    }
    float elapsedSecs = t1 - startTime;

    // convert state to string
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
    client.println("}");
  }

  void loop()
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
};

Adafruit_INA219 ina219_A(0x40);
Adafruit_INA219 ina219_B(0x41);
Adafruit_INA219 ina219_C(0x44);
Adafruit_INA219 ina219_D(0x45);

Battery batA('A', 2, 3, &ina219_A);
Battery batB('B', 4, 5, &ina219_B);
Battery batC('C', 6, 7, &ina219_C);
Battery batD('D', 8, 9, &ina219_D);

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  batA.setOutput(1, 0);
  batB.setOutput(1, 0);
  batC.setOutput(1, 0);
  batD.setOutput(1, 0);

  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }
  delay(1);
  Serial.println("Serial open!");

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid); // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(4000);
  }
  server.begin();    // start the web server on port 80
  printWifiStatus(); // you're connected now, so print out the status

  if (!batA.ina219->begin())
  {
    Serial.println("Failed to find ina219 @ 0x40 chip");
    batA.state = S_NO_INA219;
  }
  if (!batB.ina219->begin())
  {
    Serial.println("Failed to find ina219 @ 0x41 chip");
    batB.state = S_NO_INA219;
  }
  if (!batC.ina219->begin())
  {
    Serial.println("Failed to find ina219 @ 0x42 chip");
    batC.state = S_NO_INA219;
  }
  if (!batD.ina219->begin())
  {
    Serial.println("Failed to find ina219 @ 0x43 chip");
    batD.state = S_NO_INA219;
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void wifiLoop(Battery &bat1, Battery &bat2, Battery &bat3, Battery &bat4)
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client)
  {                               // if you get a client,
    Serial.println("new client"); // print a message out the serial port
    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out to the serial monitor
        if (c == '\n')
        { // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // client.println("HTTP/1.1 200 OK");
            // client.println("Content-type:text/html");
            // client.println();
            // client.println("<html>");
            // client.println("<head>");
            // client.println("  <meta charset=\"UTF-8\">");
            // client.println("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            // client.println("<style>");
            // client.println("  body {");
            // client.println("    font-family: monospace;");
            // client.println("    font-size: 14px;");
            // client.println("    width: 360px;");
            // client.println("    max-width: 100%;");
            // client.println("    margin: 4px;");
            // client.println("  }");
            // client.println("  p {");
            // client.println("    margin: 0;");
            // client.println("  }");
            // client.println("  .row {");
            // client.println("    display: flex;");
            // client.println("    justify-content: space-between;");
            // client.println("  }");
            // client.println("</style>");
            // client.println("</head>");
            // client.println("<body>");
            // bat1.printStatusHtml(client);
            // client.println("<br/>");
            // bat2.printStatusHtml(client);
            // client.println("<br/>");
            // bat3.printStatusHtml(client);
            // client.println("<br/>");
            // bat4.printStatusHtml(client);
            // client.println("</body>");
            // client.println("</html>");
            // client.println();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Access-Control-Allow-Origin: *"); // allow all origins
            client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
            client.println("Access-Control-Allow-Headers: Content-Type");
            client.println("Connection: close");
            client.println();
            client.println("[");
            bat1.printStatusJson(client);
            client.println(",");
            bat2.printStatusJson(client);
            client.println(",");
            bat3.printStatusJson(client);
            client.println(",");
            bat4.printStatusJson(client);
            client.println();
            client.println("]");
            break;
          }
          else
          { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H"))
        {
          digitalWrite(LED_BUILTIN, HIGH); // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L"))
        {
          digitalWrite(LED_BUILTIN, LOW); // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void loop(void)
{
  batA.loop();
  batB.loop();
  batC.loop();
  batD.loop();
  wifiLoop(batA, batB, batC, batD);
  delay(100);
}