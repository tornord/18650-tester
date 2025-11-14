#include "WiFiS3.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "arduino_secrets.h"
#include "Battery.h"
#include "constants.h"

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;          // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

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